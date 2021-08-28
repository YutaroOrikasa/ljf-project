
#include <string>
#include <iostream>
#include <thread>
#include <dlfcn.h>

#include "gtest/gtest.h"

#include <llvm/IR/Function.h>

#include "ljf/runtime.hpp"
#include "runtime-internal.hpp"
#include "ljf/initmsg.hpp"
#include "Object.hpp"
#include "ObjectIterator.hpp"
#include "TypeObject.hpp"

namespace ljf
{
InitMsg im{"ljf runtime loading"};
DoneMsg dm{"ljf runtime unloading"};

} // namespace ljf

namespace std
{
template <>
struct hash<ljf::TypeObject>
{
    size_t operator()(const ljf::TypeObject &type) const;
};
} // namespace std

static struct RuntimeLoadedOnceCheck
{
    RuntimeLoadedOnceCheck()
    {
        auto this_program_handle = dlopen(NULL, RTLD_LAZY);
        assert(this_program_handle);
        auto fn_addr = dlsym(this_program_handle, "ljf_call_function");
        assert(fn_addr == ljf_call_function);
    }
} runtime_loaded_once_check;

static size_t allocated_memory_size;
struct Done
{

    Done() = default;

    ~Done()
    {
        try
        {
            std::cout << "LJF: allocated_memory_size: " << allocated_memory_size << std::endl;
        }
        catch (...)
        {
            // nop
        }
    }
};
static Done done;

namespace ljf
{

using TemporaryStorage = Object;

using FunctionPtr = Object *(*)(Environment *, TemporaryStorage *);

struct FunctionData
{
    // naive means 'not optimized'
    const llvm::Function *naive_llvm_function;

    FunctionPtr naive_function = nullptr;

    struct DataForArgType
    {
        std::size_t called_count = 0;

        // specialized function for argument types
        llvm::Function *specialized_function = nullptr;
    };

    std::unordered_map<TypeObject, DataForArgType> data_for_arg_type;
};

// class Root : public Object
// {
// private:
//     std::mutex mutex;
//     std::unordered_map<std::thread::id, Object*> threads;
// public:
//     void set_thread(std::thread::id id, Object* obj) {
//         std::lock_guard lk {mutex};
//         threads[id] = obj;
//     }

//     void erase_thread(std::thread::id id) {
//         std::lock_guard lk {mutex};
//         threads.erase(id);
//     }

//     std::vector<Object*> get_all_threads() {
//         std::lock_guard lk {mutex};
//         std::vector<Object*> ret;
//         for (auto&& [id, obj] : threads)
//         {
//             ret.push_back(obj);
//         }
//         return ret;
//     }
// };

class ThreadLocalRoot;

class GlobalRoot
{
private:
    std::mutex mutex;
    std::unordered_map<std::thread::id, ThreadLocalRoot *> threads;

public:
    void add_thread(std::thread::id id, ThreadLocalRoot *thread)
    {

        std::lock_guard lk{mutex};
        threads[id] = thread;
    }

    void erase_thread(std::thread::id id)
    {
        std::lock_guard lk{mutex};
        threads.erase(id);
    }

    template <typename Function>
    void foreach_thread(Function &&f)
    {
        std::lock_guard lk{mutex};
        for (auto &&[id, thread] : threads)
        {
            (void)id;
            f(thread);
        }
    }
};

struct CallStack : Object
{
    explicit CallStack(Environment *env, TemporaryStorage *tmp, CallStack *next)
    {
        ljf::set_object_to_table(this, "env", env);
        ljf::set_object_to_table(this, "tmp", tmp);
        ljf::set_object_to_table(this, "next", next);
    }

    Environment *env()
    {
        return ljf_get(this, "env", ljf::visible);
    }

    TemporaryStorage *tmp()
    {
        return ljf_get(this, "tmp", ljf::visible);
    }

    CallStack *next()
    {
        return static_cast<CallStack *>(ljf_get(this, "next", ljf::visible));
    }
};

class ThreadLocalRoot
{
private:
    Object *returned_object_ = nullptr;
    CallStack *call_stack = nullptr;

public:
    void hold_returned_object(Object *obj)
    {
        if (obj == returned_object_)
        {
            return;
        }

        decrement_ref_count(returned_object_);
        returned_object_ = obj;
        increment_ref_count(obj);
    }

    void push_call_stack(Environment *env, TemporaryStorage *tmp)
    {
        auto next = call_stack;
        call_stack = new CallStack(env, tmp, next);
    }

    void pop_call_stack()
    {
        auto next = call_stack->next();
        call_stack = next;
    }
};

class FunctionTable
{
private:
    std::mutex mutex_;
    std::size_t size_;
    std::unordered_map<FunctionId, FunctionData> function_table_;

    FunctionId add(const FunctionData &f)
    {
        std::lock_guard lk{mutex_};
        const auto id = size_;
        size_++;
        function_table_.insert_or_assign(id, f);
        return id;
    }

public:
    FunctionId add_native(FunctionPtr fn)
    {
        return add({nullptr, fn});
    }

    FunctionId add_llvm(llvm::Function *fn)
    {
        return add({fn, nullptr});
    }

    void set_native(FunctionId id, FunctionPtr fn)
    {
        std::lock_guard lk{mutex_};
        try
        {
            function_table_.at(id).naive_function = fn;
        }
        catch (const std::out_of_range &)
        {
            throw ljf::runtime_error("error: set function to invalid function id");
        }
    }

    FunctionData &get(FunctionId id)
    {
        try
        {
            return function_table_.at(id);
        }
        catch (std::out_of_range)
        {
            throw ljf::runtime_error("no such function id");
        }
    }
};

namespace
{
FunctionTable function_table;
GlobalRoot global_root;
thread_local ThreadLocalRoot *thread_local_root = []() {
    auto th = new ThreadLocalRoot;
    global_root.add_thread(std::this_thread::get_id(), th);
    return th;
}();
struct ThreadLocalRootEraser
{
    ~ThreadLocalRootEraser()
    {
        global_root.erase_thread(std::this_thread::get_id());
    }
};
thread_local ThreadLocalRootEraser eraser;

FunctionId register_llvm_function(llvm::Function *f)
{
    return function_table.add_llvm(f);
}
} // namespace

void check_not_null(const Object *obj)
{
    if (!obj)
    {
        throw runtime_error("null pointer exception");
    }
}

} // namespace ljf

size_t std::hash<ljf::TypeObject>::operator()(const ljf::TypeObject &type) const
{
    return type.hash();
}

using namespace ljf;

namespace ljf::internal
{

ObjectHolder
create_environment(Object *arg)
{

    ObjectHolder env = ljf_new_object();

    auto env_maps = ljf_new_object();
    ljf_set_object_to_hidden_table(env.get(), "ljf.env.maps", env_maps);

    if (arg)
    {
        auto local_env = ljf_new_object();
        local_env->swap(*arg);
        // arg is now empty

        // set maps[0]
        ljf_push_object_to_array(env_maps, local_env);
    }
    return env;
}

ObjectHolder
create_environment(bool prepare_0th_frame /*=true*/)
{

    if (prepare_0th_frame)
    {
        ObjectHolder arg = ljf_new_object();
        return create_environment(arg.get());
    }
    else
    {
        return create_environment(nullptr);
    }
}
} // namespace ljf::internal

extern "C" void ljf_internal_set_native_function(FunctionId id, FunctionPtr fn)
{
    function_table.set_native(id, fn);
}

namespace
{

ObjectHolder
create_callee_environment(Environment *parent, Object *arg)
{
    // Prepare callee local env and set arguments into the local env.
    auto callee_env = internal::create_environment(arg);
    auto callee_env_maps = ljf_get_object_from_hidden_table(callee_env.get(), "ljf.env.maps");

    if (!parent)
    {
        return callee_env;
    }

    auto parent_env_maps = ljf_get_object_from_hidden_table(parent, "ljf.env.maps");

    if (!parent_env_maps)
    {
        return callee_env;
    }

    // set maps[1:]
    // parent_env_maps->dump();
    for (size_t i = 0; i < parent_env_maps->array_size(); i++)
    {
        // std::cout << "i: " << i << "\n";
        Object *map = ljf_get_object_from_array(parent_env_maps, i);
        assert(map);
        ljf_push_object_to_array(callee_env_maps, map);
    }

    return callee_env;
}

} // namespace

Object *ljf_get(Object *obj, const char *key, ljf::TableVisiblity visiblity)
{
    check_not_null(obj);

    return obj->get(key);
}

void ljf_set(Object *obj, const char *key, Object *value, ljf::TableVisiblity visiblity)
{
    check_not_null(obj);
    obj->set_object_to_table(visiblity, key, value);
}

Object *ljf_get_object_from_hidden_table(Object *obj, const char *key)
{
    check_not_null(obj);

    return obj->get_object_from_table(hidden, key);
}

void ljf_set_object_to_hidden_table(Object *obj, const char *key, Object *value)
{
    check_not_null(obj);

    obj->set_object_to_table(hidden, key, value);
}

/**************** array API ***************/

Object *ljf_get_object_from_array(Object *obj, size_t index)
{
    check_not_null(obj);

    if (index >= obj->array_size())
    {
        return ljf_undefined;
    }

    return obj->array_at(index);
}

void ljf_set_object_to_array(Object *obj, size_t index, Object *value)
{
    check_not_null(obj);

    obj->array_set_at(index, value);
}

void ljf_push_object_to_array(Object *obj, Object *value)
{
    check_not_null(obj);

    obj->array_push(value);
}

size_t ljf_array_size(Object *obj)
{
    check_not_null(obj);
    return obj->array_size();
}

//*********************//
FunctionId ljf_get_function_id_from_function_table(Object *obj, const char *key)
{
    check_not_null(obj);

    try
    {
        return obj->get_function_id(key);
    }
    catch (std::out_of_range)
    {
        throw ljf::runtime_error("function table: no such key: " + std::string(key));
    }
}

void ljf_set_function_id_to_function_table(Object *obj, const char *key, FunctionId function_id)
{
    check_not_null(obj);

    obj->set_function_id(key, function_id);
}

/// if arg == null callee's local frame env and argument env will not be created.
Object *ljf_call_function(FunctionId function_id, Environment *env, Object *arg)
{
    // std::cout << "arg\n";
    // if (arg)
    // {
    //     arg->dump();
    // }
    // else
    // {
    //     std::cout << "nullptr\n";
    // }

    auto &func_data = function_table.get(function_id);
    // std::cout << func_data.naive_llvm_function->getName().str() << "\n";

    auto callee_env = create_callee_environment(env, arg);

    auto arg_type = callee_env->calculate_type();

    auto &data_for_arg_type = func_data.data_for_arg_type[*arg_type];
    data_for_arg_type.called_count++;

    FunctionPtr func_ptr = func_data.naive_function;
    TemporaryStorage tmp;

    auto ret = func_ptr(callee_env.get(), &tmp);

    thread_local_root->hold_returned_object(ret);

    // std::cout << "END " << func_data.naive_llvm_function->getName().str() << "\n";

    return ret;
}

Object *ljf_new_object_with_native_data(uint64_t data)
{
    Object *obj = new Object(data);
    allocated_memory_size += sizeof(Object);
    thread_local_root->hold_returned_object(obj);
    return obj;
}

Object *ljf_new_object()
{
    return ljf_new_object_with_native_data(0);
}

uint64_t ljf_get_native_data(const Object *obj)
{
    check_not_null(obj);

    return obj->get_native_data();
}

Object *ljf_get_object_from_environment(Environment *env, const char *key, ljf::TableVisiblity vis)
{
    check_not_null(env);

    auto maps = ljf_get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps)
    {
        env->dump();
        throw ljf::runtime_error("ljf_get_object_from_environment: not a Environment");
    }

    for (size_t i = 0; i < maps->array_size(); i++)
    {
        // env object is nested.
        // maps->array_at(0) is most inner environment.
        auto obj = maps->array_at(i);
        auto value = ljf_get(obj, key, vis);
        if (value)
        {
            return value;
        }
    }

    return ljf_undefined;
}

void ljf_set_object_to_environment(Environment *env, const char *key, Object *value)
{
    check_not_null(env);

    auto maps = ljf_get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps)
    {
        throw ljf::runtime_error("ljf_get_object_from_environment: not a Environment");
    }

    auto map0 = maps->array_at(0);
    set_object_to_table(map0, key, value);
}

FunctionId ljf_register_native_function(Object *(*fn)(Environment *, TemporaryStorage *))
{
    return function_table.add_native(fn);
}

Object *ljf_wrap_c_str(const char *str)
{
    static_assert(sizeof(str) <= sizeof(uint64_t));
    ObjectHolder wrapper = ljf_new_object_with_native_data(reinterpret_cast<uint64_t>(str));

    // TODO: set attribute = constant
    set_object_to_table(wrapper.get(), "length", ljf_new_object_with_native_data(strlen(str)));

    thread_local_root->hold_returned_object(wrapper.get());
    return wrapper.get();
}

/// return: returned object of module_main()
/// out: env: environment of module
static Object *load_source_code(const char *language, const char *source_path, Object *&env, bool create_module_local_env)
{
    ObjectHolder env_holder = env;
    if (create_module_local_env)
    {
        ObjectHolder arg = ljf_new_object();
        env_holder = create_callee_environment(env, arg.get());
    }

    ObjectHolder ret = ljf::internal::load_source_code(language, source_path, env_holder.get());
    thread_local_root->hold_returned_object(ret.get());
    env = env_holder.get();
    return ret.get();
}

Object *ljf_load_source_code(const char *language, const char *source_path, Object *env, bool create_module_local_env)
{
    load_source_code(language, source_path, /* reference of */ env, create_module_local_env);
    // now env is updated.
    return env;
}

// ----- internal -----

FunctionId ljf_internal_register_llvm_function(llvm::Function *f)
{
    return ljf::register_llvm_function(f);
}

Object *ljf_internal_get_object_by_index(Object *obj, uint64_t index)
{
    return obj->array_table_get_index(index);
}

void ljf_internal_set_object_by_index(Object *obj, uint64_t index, Object *value)
{
    obj->array_table_set_index(index, value);
}

void ljf_internal_reserve_object_array_table_size(Object *obj, uint64_t size)
{
    obj->array_table_reserve(size);
}

void ljf_internal_resize_object_array_table_size(Object *obj, uint64_t size)
{
    obj->array_table_resize(size);
}

extern "C" int ljf_internal_start_entry_point(ljf_main_t ljf_main,
                                              const std::string &language, const std::string &source_path,
                                              int argc, const char **argv)
{
    if (ljf_main)
    {
        return ljf_main(argc, argv);
    }
    else
    {
        ObjectHolder args_holder = ljf_new_object();
        auto args = args_holder.get();
        for (size_t i = 0; i < argc; i++)
        {
            ObjectHolder wrap_holder = ljf_wrap_c_str(argv[i]);
            auto wrap = wrap_holder.get();
            ljf_push_object_to_array(args, wrap);
        }
        ObjectHolder arg = ljf_new_object();
        set_object_to_table(arg.get(), "args", args);
        ObjectHolder env_holder = create_callee_environment(nullptr, arg.get());

        Object *env = env_holder.get();
        ObjectHolder ret = load_source_code(language.c_str(), source_path.c_str(), env, false);
        if (ret == ljf_undefined)
        {
            return 0;
        }
        return ljf_get_native_data(ret.get());
    }
}

namespace ljf::internal::check_
{
ljf_internal_start_entry_point_t ljf_internal_start_entry_point_ = ljf_internal_start_entry_point;
} // namespace ljf::internal::check_

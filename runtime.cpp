
#include <string>
#include <iostream>
#include <forward_list>
#include <list>
#include <thread>
#include <mutex>
#include <algorithm>
#include <stdexcept>

#include <llvm/IR/Function.h>

#include "ljf/runtime.hpp"
#include "runtime-internal.hpp"
#include "ljf/initmsg.hpp"

namespace ljf
{
InitMsg im{"ljf runtime loading"};
DoneMsg dm{"ljf runtime unloading"};

class Object;
using ObjectPtr = Object *;

using FunctionId = LJFFunctionId;

struct TypeObject;

} // namespace ljf

namespace std
{
template <>
struct hash<ljf::TypeObject>
{
    size_t operator()(const ljf::TypeObject &typ) const noexcept
    {
        return 0;
    }
};

} // namespace std

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

std::shared_ptr<TypeObject> calculate_type(const Object &obj);

enum TableVisiblity
{
    visible,
    hidden
};

class Object
{
private:
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<ObjectPtr> array_table_;
    std::vector<ObjectPtr> array_;
    std::unordered_map<std::string, FunctionId> function_id_table_;
    using native_data_t = std::uint64_t;
    const native_data_t native_data_ = 0;
    ssize_t ref_count_ = 0;

public:
    Object() = default;
    explicit Object(native_data_t data) : native_data_(data) {}
    Object(const Object &) = delete;
    Object(Object &&) = delete;
    Object &operator=(const Object &) = delete;
    Object &operator=(Object &&) = delete;

    Object *get(const std::string &key)
    {
        std::lock_guard lk{mutex_};

        if (!hash_table_.count(key))
        {
            // std::cerr << "ljf get(): no such key: " << key << std::endl;
            return ljf_undefined;
        }

        return array_table_.at(hash_table_.at(key));
    }
    // void set(const std::string &key, ObjectPtr value)
    // {
    //     hash_table_.insert_or_assign(key, value);
    // }

    void set_object_to_table(TableVisiblity visiblity, const char *key, Object *value)
    {

        auto &table = (visiblity == visible) ? hash_table_ : hidden_table_;
        Object *old_value = nullptr;
        {
            std::lock_guard lk{mutex_};
            if (table.count(key))
            {
                size_t index = table.at(key);

                Object *&elem_ref = array_table_.at(index);
                old_value = elem_ref;
                elem_ref = value;
            }
            else
            {
                size_t index = array_table_.size();
                array_table_.push_back(value);
                table[key] = index;
            }
        }
        // assert(value != 0);
        // assert(value);

        increment_ref_count(value);

        decrement_ref_count(old_value);
    }
    Object *get_object_from_table(TableVisiblity visiblity, const char *key)
    {

        auto &table = (visiblity == visible) ? hash_table_ : hidden_table_;
        std::lock_guard lk{mutex_};

        return array_table_.at(table.at(key));
    }

    FunctionId get_function_id(const std::string &key)
    {
        std::lock_guard lk{mutex_};
        return function_id_table_.at(key);
    }
    void set_function_id(const std::string &key, FunctionId function_id)
    {
        function_id_table_.insert_or_assign(key, function_id);
    }

    void lock()
    {
        mutex_.lock();
    }

    void unlock()
    {
        mutex_.unlock();
    }

    // array API
    size_t array_size()
    {
        std::lock_guard lk{mutex_};
        return array_.size();
    }
    Object *array_at(uint64_t index)
    {
        std::lock_guard lk{mutex_};
        return array_.at(index);
    }
    void array_set_at(uint64_t index, Object *value)
    {
        Object *old_value;
        {
            std::lock_guard lk{mutex_};
            auto &elem_ref = array_.at(index);
            old_value = elem_ref;
            elem_ref = value;
        }
        assert(value); // DEBUG
        increment_ref_count(value);
        decrement_ref_count(old_value);
    }

    void array_push(Object *value)
    {
        {
            std::lock_guard lk{mutex_};
            array_.push_back(value);
        }
        // assert(value); // DEBUG
        increment_ref_count(value);
    }

    // native data
    uint64_t get_native_data() const
    {
        return native_data_;
    }

    std::shared_ptr<TypeObject> calculate_type()
    {
        std::lock_guard lk{mutex_};
        if (type_object_)
        {
            return type_object_;
        }

        type_object_ = ljf::calculate_type(*this);
        return type_object_;
    }

    ~Object()
    {
        // std::cout << "~Object() " << this << " dump\n";
        // dump();

        for (auto &&obj : array_table_)
        {
            decrement_ref_count(obj);
        }

        for (auto &&obj : array_)
        {
            decrement_ref_count(obj);
        }
    }

    friend void increment_ref_count(Object *obj);
    friend void decrement_ref_count(Object *obj);

    void dump()
    {
        std::cout << "{\n";
        std::cout << "    {\n";
        for (auto &&[key, value] : hash_table_)
        {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    hidden: {\n";
        for (auto &&[key, value] : hidden_table_)
        {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    array table: [";
        for (auto &&v : array_table_)
        {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    array: [";
        for (auto &&v : array_)
        {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    native_data: " << native_data_ << "\n";
        std::cout << "    ref_count: " << ref_count_ << "\n";

        std::cout << "}\n";
    }
};

void increment_ref_count(Object *obj)
{
    if (obj)
    {
        std::lock_guard lk{*obj};
        obj->ref_count_++;
    }
}

void decrement_ref_count(Object *obj)
{
    if (!obj)
    {
        return;
    }

    obj->lock();
    assert(obj->ref_count_ > 0);
    obj->ref_count_--;
    if (obj->ref_count_ == 0)
    {
        obj->unlock();
        delete obj;

        assert(allocated_memory_size >= sizeof(Object));
        allocated_memory_size -= sizeof(Object);
        return;
    }
    obj->unlock();
}

ObjectHolder::ObjectHolder(Object *o) noexcept : obj_(o)
{
    increment_ref_count(obj_);
}

ObjectHolder::ObjectHolder(const ObjectHolder &other) noexcept : ObjectHolder(other.obj_) {}

// On move we don't have to operate refcount
ObjectHolder::ObjectHolder(ObjectHolder &&other) noexcept : obj_(other.obj_)
{
    other.obj_ = nullptr;
}

ObjectHolder &ObjectHolder::operator=(Object *o) noexcept
{
    decrement_ref_count(obj_);
    obj_ = o;
    increment_ref_count(obj_);
    return *this;
}

ObjectHolder &ObjectHolder::operator=(const ObjectHolder &other) noexcept
{
    return (*this = other.obj_);
}
ObjectHolder &ObjectHolder::operator=(ObjectHolder &&other) noexcept
{
    decrement_ref_count(obj_);
    obj_ = other.obj_;
    other.obj_ = nullptr;
    return *this;
}

ObjectHolder::~ObjectHolder()
{
    decrement_ref_count(obj_);
}

using TemporaryStorage = Object;

struct TypeObject
{
    /* data */
    bool operator==(const TypeObject &other) const
    {
        return true;
    };
};
std::shared_ptr<TypeObject> calculate_type(const Object &obj)
{
    return std::make_shared<TypeObject>();
}

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
    void set_thread(std::thread::id id, ThreadLocalRoot *thread)
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
        ljf_set_object_to_table(this, "env", env);
        ljf_set_object_to_table(this, "tmp", tmp);
        ljf_set_object_to_table(this, "next", next);
    }

    Environment *env()
    {
        return ljf_get_object_from_table(this, "env");
    }

    TemporaryStorage *tmp()
    {
        return ljf_get_object_from_table(this, "tmp");
    }

    CallStack *next()
    {
        return static_cast<CallStack *>(ljf_get_object_from_table(this, "next"));
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

// class CallFrameData {
//     Object obj;
//     Root& root_;

// public:
//     explicit
//     CallFrameData(Root& root) : root_(root) {
//         auto thread_id = std::this_thread::get_id();
//         root.set_thread(thread_id, &obj);
//     }

//     ~CallFrameData() {
//         auto thread_id = std::this_thread::get_id();
//         root_.erase_thread(thread_id);
//     }

//     void hold_returned_object(Object* obj) {

//     }
// };

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
    global_root.set_thread(std::this_thread::get_id(), th);
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

} // namespace

FunctionId register_llvm_function(llvm::Function *f)
{
    return function_table.add_llvm(f);
}

void check_not_null(const Object *obj)
{
    if (!obj)
    {
        throw runtime_error("null pointer exception");
    }
}

} // namespace ljf

using namespace ljf;

namespace ljf::internal
{

ObjectHolder
create_environment(bool prepare_0th_frame /*=true*/)
{

    ObjectHolder env = ljf_new_object();

    auto env_maps = ljf_new_object();
    ljf_set_object_to_hidden_table(env.get(), "ljf.env.maps", env_maps);

    if (prepare_0th_frame)
    {
        // set maps[0]
        ljf_push_object_to_array(env_maps, ljf_new_object());
    }
    return env;
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
    auto callee_env = internal::create_environment(bool(arg));
    auto callee_env_maps = ljf_get_object_from_hidden_table(callee_env.get(), "ljf.env.maps");

    if (arg)
    {
        // set maps[1]
        ljf_push_object_to_array(callee_env_maps, arg);
    }

    if (!parent)
    {
        return callee_env;
    }

    auto parent_env_maps = ljf_get_object_from_hidden_table(parent, "ljf.env.maps");

    if (!parent_env_maps)
    {
        return callee_env;
    }

    // set maps[2:]
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

// void set_object_to_table(Object &obj, std::unordered_map<std::string, Object *> &table, const char *key, Object *value)
// {

//     Object *old_value;
//     {
//         std::lock_guard lk{obj};

//         Object *&elem_ref = table[key];
//         old_value = elem_ref;
//         elem_ref = value;
//     }

//     decrement_ref_count(old_value);
// }
} // namespace

Object *ljf_get_object_from_table(Object *obj, const char *key)
{
    check_not_null(obj);

    return obj->get(key);
}

void ljf_set_object_to_table(Object *obj, const char *key, Object *value)
{
    check_not_null(obj);
    obj->set_object_to_table(visible, key, value);
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

size_t ljf_array_size(LJFObject *obj)
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

Object *ljf_get_object_from_environment(Environment *env, const char *key)
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
        auto obj = maps->array_at(i);
        auto value = ljf_get_object_from_table(obj, key);
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
    ljf_set_object_to_table(map0, key, value);
}

FunctionId ljf_register_native_function(Object *(*fn)(Environment *, TemporaryStorage *))
{
    return function_table.add_native(fn);
}

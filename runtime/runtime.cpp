
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <thread>

#include "gtest/gtest.h"

#include <llvm/ADT/ScopeExit.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include "Object.hpp"
#include "ObjectIterator.hpp"
#include "Roots.hpp"
#include "TypeObject.hpp"
#include "ljf/initmsg.hpp"
#include "ljf/runtime.hpp"
#include "runtime-internal.hpp"

namespace ljf {
InitMsg im{"ljf runtime loading"};
DoneMsg dm{"ljf runtime unloading"};

} // namespace ljf

namespace std {} // namespace std

static struct RuntimeLoadedOnceCheck {
    RuntimeLoadedOnceCheck() {
        auto this_program_handle = dlopen(NULL, RTLD_LAZY);
        assert(this_program_handle);
        auto fn_addr = dlsym(this_program_handle, "ljf_call_function");
        assert(fn_addr == ljf_call_function);
    }
} runtime_loaded_once_check;

static size_t allocated_memory_size;
struct Done {

    Done() = default;

    ~Done() {
        try {
            std::cout << "LJF: allocated_memory_size: " << allocated_memory_size
                      << std::endl;
        } catch (...) {
            // nop
        }
    }
};
static Done done;

namespace ljf {

class TemporaryHolders {
private:
    std::vector<ObjectHolder> holders_;

public:
    Object *add(Object *obj) {
        holders_.push_back(obj);
        return obj;
    }
};

class Context {
private:
    TemporaryHolders temporary_holders_;
    llvm::Module *LLVMModule_;
    Context *caller_context_ = nullptr;

public:
    explicit Context(llvm::Module *LLVMModule, Context *caller_context)
        : LLVMModule_(LLVMModule), caller_context_(caller_context) {}
    Object *register_temporary_object(Object *obj) {
        return temporary_holders_.add(obj);
    }
};

using TemporaryStorage = Object;

struct FunctionData {
    // naive means 'not optimized'
    const llvm::Function *naive_llvm_function;
    llvm::Module *LLVMModule;

    FunctionPtr naive_function = nullptr;

    struct DataForArgType {
        std::size_t called_count = 0;

        // specialized function for argument types
        llvm::Function *specialized_function = nullptr;
    };

    std::unordered_map<TypeObject, DataForArgType> data_for_arg_type;
};

class FunctionTable {
private:
    std::mutex mutex_;
    std::size_t size_;
    std::unordered_map<FunctionId, FunctionData> function_table_;

    FunctionId add(const FunctionData &f) {
        std::lock_guard lk{mutex_};
        const auto id = size_;
        size_++;
        function_table_.insert_or_assign(id, f);
        return id;
    }

public:
    FunctionId add_native(FunctionPtr fn) {
        return add({nullptr, nullptr, fn});
    }

    FunctionId add_llvm(llvm::Function *fn, llvm::Module *module) {
        return add({fn, module, nullptr});
    }

    void set_native(FunctionId id, FunctionPtr fn) {
        std::lock_guard lk{mutex_};
        try {
            function_table_.at(id).naive_function = fn;
        } catch (const std::out_of_range &) {
            throw ljf::runtime_error(
                "error: set function to invalid function id");
        }
    }

    FunctionData &get(FunctionId id) {
        try {
            return function_table_.at(id);
        } catch (std::out_of_range) {
            throw ljf::runtime_error("no such function id");
        }
    }
};

namespace {
    FunctionTable function_table;

    FunctionId register_llvm_function(llvm::Function *f, llvm::Module *module) {
        return function_table.add_llvm(f, module);
    }
} // namespace

// Roots
namespace {
    GlobalRoot global_root;
    thread_local ThreadLocalRoot *thread_local_root = []() {
        auto th = new ThreadLocalRoot;
        global_root.add_thread(std::this_thread::get_id(), th);
        return th;
    }();
    struct ThreadLocalRootEraser {
        ~ThreadLocalRootEraser() {
            global_root.erase_thread(std::this_thread::get_id());
            delete thread_local_root;
        }
    };
    thread_local ThreadLocalRootEraser eraser;

} // namespace

void check_not_null(const Object *obj) {
    if (!obj) {
        throw runtime_error("null pointer exception");
    }
}

} // namespace ljf

using namespace ljf;

namespace ljf::internal {

ObjectHolder create_environment(Object *arg) {

    ObjectHolder env = ljf_new_object();

    ObjectHolder env_maps = ljf_new_object();
    set_object_to_hidden_table(env.get(), "ljf.env.maps", env_maps);

    if (arg) {
        auto local_env = ljf_new_object();
        local_env->swap(*arg);
        // arg is now empty

        // set maps[0]
        ljf_array_push(env_maps, local_env);
    }
    return env;
}

ObjectHolder create_environment(bool prepare_0th_frame /*=true*/) {

    if (prepare_0th_frame) {
        ObjectHolder arg = ljf_new_object();
        return create_environment(arg.get());
    } else {
        return create_environment(nullptr);
    }
}

ObjectHolder create_callee_environment(Environment *parent, Object *arg) {
    // Prepare callee local env and set arguments into the local env.
    auto callee_env = internal::create_environment(arg);
    auto callee_env_maps =
        get_object_from_hidden_table(callee_env.get(), "ljf.env.maps");

    if (!parent) {
        return callee_env;
    }

    auto parent_env_maps = get_object_from_hidden_table(parent, "ljf.env.maps");

    if (!parent_env_maps) {
        return callee_env;
    }

    // set maps[1:]
    // parent_env_maps->dump();
    for (size_t i = 0; i < parent_env_maps->array_size(); i++) {
        // std::cout << "i: " << i << "\n";
        Object *map = ljf_array_get(parent_env_maps, i);
        assert(map);
        ljf_array_push(callee_env_maps, map);
    }

    return callee_env;
}

} // namespace ljf::internal

extern "C" {
using namespace ljf::internal;

void ljf_internal_set_native_function(FunctionId id, FunctionPtr fn) {
    function_table.set_native(id, fn);
}

/**************** table API ***************/
Object *ljf_get(Object *obj, void *key, LJFAttribute attr) {
    check_not_null(obj);

    return obj->get(key, attr);
}

void ljf_set(Object *obj, void *key, Object *value, LJFAttribute attr) {
    check_not_null(obj);
    obj->set(key, value, attr);
}

/**************** array API ***************/

Object *ljf_array_get(Object *obj, size_t index) {
    check_not_null(obj);

    if (index >= obj->array_size()) {
        return ljf_undefined;
    }

    return obj->array_at(index);
}

void ljf_array_set(Object *obj, size_t index, Object *value) {
    check_not_null(obj);

    obj->array_set_at(index, value);
}

void ljf_array_push(Object *obj, Object *value) {
    check_not_null(obj);

    obj->array_push(value);
}

size_t ljf_array_size(Object *obj) {
    check_not_null(obj);
    return obj->array_size();
}

//*********************//
FunctionId ljf_get_function_id_from_function_table(Object *obj,
                                                   const char *key) {
    check_not_null(obj);

    try {
        return obj->get_function_id(key);
    } catch (std::out_of_range) {
        throw ljf::runtime_error("function table: no such key: " +
                                 std::string(key));
    }
}

void ljf_set_function_id_to_function_table(Object *obj, const char *key,
                                           FunctionId function_id) {
    check_not_null(obj);

    obj->set_function_id(key, function_id);
}

/// if arg == null callee's local frame env and argument env will not be
/// created.
Object *ljf_call_function(Context *caller_ctx, FunctionId function_id,
                          Environment *env, Object *arg) {
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
    Context ctx{func_data.LLVMModule, caller_ctx};

    thread_local_root->set_top_context(&ctx);
    auto finally_restore_ctx = llvm::make_scope_exit(
        [&caller_ctx] { thread_local_root->set_top_context(caller_ctx); });

    auto ret = func_ptr(&ctx, callee_env.get());

    thread_local_root->hold_returned_object(ret);

    // std::cout << "END " << func_data.naive_llvm_function->getName().str() <<
    // "\n";

    return ret;
}

Object *ljf_new_object_with_native_data(uint64_t data) {
    Object *obj = new Object(data);
    allocated_memory_size += sizeof(Object);
    thread_local_root->hold_returned_object(obj);
    return obj;
}

Object *ljf_new(Context *ctx) {
    Object *obj = new Object();
    allocated_memory_size += sizeof(Object);
    return ctx->register_temporary_object(obj);
}

Object *ljf_new_object() { return ljf_new_object_with_native_data(0); }

uint64_t ljf_get_native_data(const Object *obj) {
    check_not_null(obj);

    return obj->get_native_data();
}

Object *ljf_environment_get(Environment *env, void *key, LJFAttribute attr) {
    check_not_null(env);

    auto maps = get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps) {
        throw ljf::runtime_error(
            "ljf_get_object_from_environment: not an Environment");
    }

    for (size_t i = 0; i < maps->array_size(); i++) {
        // env object is nested.
        // maps->array_at(0) is most inner environment.
        auto obj = maps->array_at(i);
        auto value = ljf_get(obj, key, attr);
        if (value) {
            return value;
        }
    }

    return ljf_undefined;
}

void ljf_environment_set(Environment *env, void *key, Object *value,
                         LJFAttribute attr) {
    check_not_null(env);

    auto maps = get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps) {
        throw ljf::runtime_error(
            "ljf_get_object_from_environment: not a Environment");
    }

    auto map0 = maps->array_at(0);
    ljf_set(map0, key, value, attr);
}

FunctionId ljf_register_native_function(FunctionPtr fn) {
    return function_table.add_native(fn);
}

Object *ljf_wrap_c_str(Context *, const char *str) {
    static_assert(sizeof(str) <= sizeof(uint64_t));
    ObjectHolder wrapper =
        ljf_new_object_with_native_data(reinterpret_cast<uint64_t>(str));

    // TODO: set attribute = constant
    set_object_to_table(wrapper.get(), "length",
                        ljf_new_object_with_native_data(strlen(str)));

    thread_local_root->hold_returned_object(wrapper.get());
    return wrapper.get();
}

/// return: returned object of module_main()
/// out: env: environment of module
static Object *load_source_code(const char *language, const char *source_path,
                                Object *&env, bool create_module_local_env) {
    ObjectHolder env_holder = env;
    if (create_module_local_env) {
        ObjectHolder arg = ljf_new_object();
        env_holder = create_callee_environment(env, arg.get());
    }

    ObjectHolder ret = ljf::internal::load_source_code(language, source_path,
                                                       env_holder.get());
    thread_local_root->hold_returned_object(ret.get());
    env = env_holder.get();
    return ret.get();
}

Object *ljf_load_source_code(const char *language, const char *source_path,
                             Object *env, bool create_module_local_env) {
    load_source_code(language, source_path, /* reference of */ env,
                     create_module_local_env);
    // now env is updated.
    return env;
}

// ----- internal -----

FunctionId ljf_internal_register_llvm_function(llvm::Function *f,
                                               llvm::Module *module) {
    return ljf::register_llvm_function(f, module);
}

Object *ljf_internal_get_object_by_index(Object *obj, uint64_t index) {
    return obj->array_table_get_index(index);
}

void ljf_internal_set_object_by_index(Object *obj, uint64_t index,
                                      Object *value) {
    obj->array_table_set_index(index, value);
}

void ljf_internal_reserve_object_array_table_size(Object *obj, uint64_t size) {
    obj->array_table_reserve(size);
}

void ljf_internal_resize_object_array_table_size(Object *obj, uint64_t size) {
    obj->array_table_resize(size);
}

int ljf_internal_start_entry_point(ljf_main_t ljf_main,
                                   const std::string &language,
                                   const std::string &source_path, int argc,
                                   const char **argv) {

    auto ctx_up = std::make_unique<Context>(nullptr, nullptr);
    auto ctx = ctx_up.get();
    if (ljf_main) {
        return ljf_main(argc, argv);
    } else {
        auto args = ljf_new(ctx);
        for (size_t i = 0; i < argc; i++) {
            ObjectHolder wrap_holder = ljf_wrap_c_str(ctx, argv[i]);
            auto wrap = wrap_holder.get();
            ljf_array_push(args, wrap);
        }
        ObjectHolder arg = ljf_new_object();
        set_object_to_table(arg.get(), "args", args);
        ObjectHolder env_holder = create_callee_environment(nullptr, arg.get());

        Object *env = env_holder.get();
        ObjectHolder ret =
            load_source_code(language.c_str(), source_path.c_str(), env, false);
        if (ret == ljf_undefined) {
            return 0;
        }
        return ljf_get_native_data(ret.get());
    }
}
}
namespace ljf::internal::check_ {
ljf_internal_start_entry_point_t ljf_internal_start_entry_point_ =
    ljf_internal_start_entry_point;
} // namespace ljf::internal::check_

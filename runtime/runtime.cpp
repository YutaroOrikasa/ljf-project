
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
#include "ljf-system-property.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/initmsg.hpp"
#include "ljf/runtime.hpp"
#include "runtime-internal.hpp"

static_assert(sizeof(LJFHandle) >= sizeof(void *));
static_assert(sizeof(LJFHandle) >= sizeof(int64_t));

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

    FunctionId add_llvm(llvm::Function *fn, llvm::Module *module,
                        FunctionPtr fn_ptr) {
        return add({fn, module, fn_ptr});
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

} // namespace ljf

using namespace ljf;

namespace ljf::internal {

/// @brief If arg_h is ljf_undefined, this function will not move argument to
/// new env.
/// @param ctx
/// @param arg_h
/// @return
ObjectHolder create_environment_with_argument(Context *ctx, LJFHandle arg_h) {

    auto env = ctx->get_from_handle(ljf_new(ctx));

    auto env_maps = ctx->get_from_handle(ljf_new(ctx));
    set_object_to_hidden_table(env, "ljf.env.maps", env_maps);
    if (arg_h != ljf_internal_null_handle) {
        auto arg = ctx->get_from_handle(arg_h);
        auto local_env = ctx->get_from_handle(ljf_new(ctx));
        local_env->swap(*arg);
        // arg is now empty

        // set maps[0]
        env_maps->array_push(local_env);
    }
    return env;
}

ObjectHolder create_environment(Context *ctx,
                                bool prepare_0th_frame /*=true*/) {

    if (prepare_0th_frame) {
        auto arg = ljf_new(ctx);
        return create_environment_with_argument(ctx, arg);
    } else {
        return create_environment_with_argument(ctx, ljf_internal_null_handle);
    }
}

ObjectHolder create_callee_environment(Environment *parent, Object *arg) {
    auto ctx = internal::make_temporary_context();
    // Prepare callee local env and set arguments into the local env.
    auto callee_env = internal::create_environment(ctx.get(), arg);
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
        Object *map = parent_env_maps->array_at(i);
        assert(map);
        callee_env_maps->array_push(map);
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
LJFHandle ljf_get(ljf::Context *ctx, LJFHandle obj, LJFHandle key,
                  LJFAttribute attr) {

    auto key_ptr = [&]() -> const void * {
        if (AttributeTraits::mask(attr, LJF_ATTR_KEY_TYPE_MASK) ==
            LJF_ATTR_C_STR_KEY) {
            return reinterpret_cast<const void *>(key);
        } else {
            // assert((attr & LJF_ATTR_KEY_TYPE_MASK) == C_STR_KEY)
            return ctx->get_from_handle(obj);
        }
    }();
    return ctx->register_temporary_object(
        ctx->get_from_handle(obj)->get(key_ptr, attr));
}

void ljf_set(Context *ctx, LJFHandle obj, LJFHandle key_handle_or_cstr,
             LJFHandle value, LJFAttribute attr) {
    assert(AttributeTraits::mask(attr, LJF_ATTR_DATA_TYPE_MASK) ==
           LJF_ATTR_BOXED_OBJECT);
    auto key = [&]() -> void * {
        if (AttributeTraits::mask(attr, LJF_ATTR_KEY_TYPE_MASK) ==
            LJF_ATTR_C_STR_KEY) {
            return reinterpret_cast<void *>(key_handle_or_cstr);
        } else {
            return ctx->get_from_handle(key_handle_or_cstr);
        }
    }();
    ctx->get_from_handle(obj)->set(key, ctx->get_from_handle(value), attr);
}

/**************** array API ***************/

LJFHandle ljf_array_get(Context *ctx, LJFHandle obj_h, size_t index) {

    auto obj = ctx->get_from_handle(obj_h);
    if (index >= obj->array_size()) {
        throw std::out_of_range("ljf_array_get");
    }

    return ctx->register_temporary_object(obj->array_at(index));
}

void ljf_array_set(Object *obj, size_t index, Object *value) {

    obj->array_set_at(index, value);
}

void ljf_array_push(Context *ctx, LJFHandle obj, LJFHandle value) {
    auto obj_raw = ctx->get_from_handle(obj);
    auto value_raw = ctx->get_from_handle(value);
    obj_raw->array_push(value_raw);
}

size_t ljf_array_size(Context *ctx, LJFHandle obj_h) {
    return ctx->get_from_handle(obj_h)->array_size();
}

//*********************//
FunctionId ljf_get_function_id_from_function_table(Object *obj,
                                                   const char *key) {

    try {
        return obj->get_function_id(key);
    } catch (std::out_of_range) {
        throw ljf::runtime_error("function table: no such key: " +
                                 std::string(key));
    }
}

void ljf_set_function_id_to_function_table(Object *obj, const char *key,
                                           FunctionId function_id) {

    obj->set_function_id(key, function_id);
}

/// if arg == null callee's local frame env and argument env will not be
/// created.
LJFHandle ljf_call_function(Context *caller_ctx, FunctionId function_id,
                            LJFHandle env, LJFHandle arg) {
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

    auto callee_env = create_callee_environment(
        caller_ctx->get_from_handle(env), caller_ctx->get_from_handle(arg));

    auto arg_type = callee_env->calculate_type();

    auto &data_for_arg_type = func_data.data_for_arg_type[*arg_type];
    data_for_arg_type.called_count++;

    FunctionPtr func_ptr = func_data.naive_function;
    Context ctx{func_data.LLVMModule, caller_ctx};

    thread_local_root->set_top_context(&ctx);
    auto finally_restore_ctx = llvm::make_scope_exit(
        [&caller_ctx] { thread_local_root->set_top_context(caller_ctx); });

    auto ret = func_ptr(&ctx, callee_env.get());

    // std::cout << "END " << func_data.naive_llvm_function->getName().str() <<
    // "\n";

    auto ret_raw = ctx.get_from_handle(ret);
    return caller_ctx->register_temporary_object(ret_raw);
}

LJFHandle ljf_new_with_native_data(Context *ctx, uint64_t data) {
    Object *obj = new Object(data);
    allocated_memory_size += sizeof(Object);
    return ctx->register_temporary_object(obj);
}

LJFHandle ljf_new(Context *ctx) { return ljf_new_with_native_data(ctx, 0); }

uint64_t ljf_get_native_data(const Object *obj) {

    return obj->get_native_data();
}

LJFHandle ljf_environment_get(ljf::Context *ctx, Environment *env,
                              LJFHandle key_handle, LJFAttribute attr) {

    auto maps = get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps) {
        throw ljf::runtime_error(
            "ljf_get_object_from_environment: not an Environment");
    }

    auto key = ctx->get_key_from_handle(key_handle, attr);
    for (size_t i = 0; i < maps->array_size(); i++) {
        // env object is nested.
        // maps->array_at(0) is most inner environment.
        auto obj = maps->array_at(i);
        try {
            return ctx->register_temporary_object(obj->get(key, attr));
        } catch (const std::out_of_range &e) {
            continue;
        }
    }

    throw std::out_of_range("LJF environment: key not found");
}

void ljf_environment_set(ljf::Context *ctx, Environment *env, LJFHandle key,
                         LJFHandle value, LJFAttribute attr) {

    auto maps = get_object_from_hidden_table(env, "ljf.env.maps");

    if (!maps) {
        throw ljf::runtime_error(
            "ljf_get_object_from_environment: not a Environment");
    }

    auto map0 = maps->array_at(0);
    map0->set(ctx->get_key_from_handle(key, attr), ctx->get_from_handle(value),
              attr);
}

FunctionId ljf_register_native_function(FunctionPtr fn) {
    return function_table.add_native(fn);
}

FunctionId ljf_register_llvm_function(Context *ctx, const char *function_name,
                                      FunctionPtr fn_ptr) {
    auto LLVMModule = ctx->get_llvm_module();
    auto fn = LLVMModule->getFunction(function_name);
    assert(fn && "no such llvm function");
    return function_table.add_llvm(fn, LLVMModule, fn_ptr);
}

LJFHandle ljf_wrap_c_str(Context *ctx, const char *str) {
    static_assert(sizeof(str) <= sizeof(uint64_t));
    auto str_obj = make_new_wrapped_object();

    set_ljf_native_system_property(str_obj, ljf_native_value_c_str,
                                   cast_to_ljf_handle(str));

    auto int_obj = make_new_wrapped_object();
    set_ljf_native_system_property(int_obj, ljf_native_value_c_str,
                                   cast_to_ljf_handle(strlen(str)));

    set_ljf_system_property(str_obj, ljf_c_str_length, int_obj);

    return ctx->register_temporary_object(str_obj.get_wrapped_pointer());

    // return wrapper.get();
}

/// return: returned object of module_main()
/// out: env: environment of module
static Object *load_source_code(const char *language, const char *source_path,
                                Object *&env, bool create_module_local_env) {
    ObjectHolder env_holder = env;
    if (create_module_local_env) {
        auto arg = make_new_held_object();
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
    return function_table.add_llvm(f, module, nullptr);
}

// disable unsafe api
// Object *ljf_internal_get_object_by_index(Object *obj, uint64_t index) {
//     return obj->array_table_get_index(index);
// }

// void ljf_internal_set_object_by_index(Object *obj, uint64_t index,
//                                       Object *value) {
//     obj->array_table_set_index(index, value);
// }

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

    auto ctx_up = internal::make_temporary_context();
    auto ctx = ctx_up.get();
    if (ljf_main) {
        return ljf_main(argc, argv);
    } else {
        auto args = ljf_new(ctx);
        for (size_t i = 0; i < argc; i++) {
            auto str = ljf_wrap_c_str(ctx, argv[i]);
            ljf_array_push(ctx, args, str);
        }
        auto arg = ljf_new(ctx);
        auto attr =
            AttributeTraits::or_attr(LJF_ATTR_MUTABLE, LJF_ATTR_BOXED_OBJECT,
                                     LJF_ATTR_VISIBLE, LJF_ATTR_C_STR_KEY);
        ljf_set(ctx, arg, cast_to_ljf_handle("args"), args, attr);
        ObjectHolder env_holder =
            create_callee_environment(nullptr, ctx->get_from_handle(arg));

        Object *env = env_holder.get();
        ObjectHolder ret =
            load_source_code(language.c_str(), source_path.c_str(), env, false);
        assert(ret != nullptr);
        return ljf_get_native_data(ret.get());
    }
}
}
namespace ljf::internal::check_ {
ljf_internal_start_entry_point_t ljf_internal_start_entry_point_ =
    ljf_internal_start_entry_point;
} // namespace ljf::internal::check_

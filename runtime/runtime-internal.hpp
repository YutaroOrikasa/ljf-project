#pragma once

#include "AttributeTraits.hpp"
#include "ObjectHolder.hpp"
#include <ljf/ljf.hpp>
#include <ljf/runtime.hpp>

#include <deque>

namespace llvm {
class Function;
} // namespace llvm

namespace ljf {

// TODO Write why Context and holding object pointer is required.
class Context {
private:
    class TemporaryHolders {
    private:
        std::deque<ObjectHolder> holders_;

    public:
        ObjectHolder *add(IncrementedObjectPtrOrNativeValue &&obj) {
            holders_.push_back(std::move(obj));
            return &holders_.back();
        }

        ObjectHolder *add(Object *obj) {
            holders_.push_back(obj);
            return &holders_.back();
        }
    };
    TemporaryHolders temporary_holders_;
    llvm::Module *LLVMModule_;
    Context *caller_context_ = nullptr;

public:
    explicit Context(llvm::Module *LLVMModule, Context *caller_context)
        : LLVMModule_(LLVMModule), caller_context_(caller_context) {}

    // On this implementation, LJFHandle is address of local ObjectHolder in
    // TemporaryHolders
    LJFHandle
    register_temporary_object(IncrementedObjectPtrOrNativeValue &&obj) {
        return reinterpret_cast<LJFHandle>(
            temporary_holders_.add(std::move(obj)));
    }

    // On this implementation, LJFHandle is address of local ObjectHolder in
    // TemporaryHolders
    LJFHandle register_temporary_object(Object *obj) {
        return reinterpret_cast<LJFHandle>(temporary_holders_.add(obj));
    }

    Object *get_from_handle(LJFHandle handle) {
        return reinterpret_cast<ObjectHolder *>(handle)->get();
    }

    const void *get_key_from_handle(LJFHandle handle, LJFAttribute attr) {
        if (AttributeTraits::mask(attr, LJFAttribute::KEY_TYPE_MASK) ==
            LJFAttribute::C_STR_KEY) {
            return const_cast<const void *>(reinterpret_cast<void *>(handle));
        } else {
            return reinterpret_cast<ObjectHolder *>(handle)->get();
        }
    }

    llvm::Module *get_llvm_module() const { return LLVMModule_; }

    Context *get_caller_context() const { return caller_context_; }
};

} // namespace ljf

namespace ljf::internal {

/// @brief We require using ljf_internal_nullptr instead of nullptr for easily
/// maintenance.
/// @details Internally and externally we prohibit using nullptr as <Object *>
/// for avoiding confusing.
/// Semantics of ljf_internal_nullptr will be changed per context.
/// Definition of ljf_internal_nullptr is allowed to be non nullptr such as some
/// an invalid address.
constexpr Object *ljf_internal_nullptr = nullptr;

inline std::unique_ptr<Context> make_temporary_context() {
    return std::make_unique<Context>(nullptr, nullptr);
}

ObjectHolder create_environment(Context *, bool prepare_0th_frame = true);
ObjectHolder create_callee_environment(Environment *parent, Object *arg);

ObjectHolder load_source_code(const std::string &language,
                              const std::string &source_path, Object *env);
} // namespace ljf::internal

extern "C" {
ljf::FunctionId ljf_internal_register_llvm_function(llvm::Function *f,
                                                    llvm::Module *module);

ljf::Object *ljf_internal_get_object_by_index(ljf::Object *obj, uint64_t index);
void ljf_internal_set_object_by_index(ljf::Object *obj, uint64_t index,
                                      ljf::Object *value);
void ljf_internal_reserve_object_array_table_size(ljf::Object *obj,
                                                  uint64_t size);
void ljf_internal_resize_object_array_table_size(ljf::Object *obj,
                                                 uint64_t size);
}

namespace ljf {
// called with dlopen and dlsym
typedef void (*ljf_internal_initialize_t)(const CompilerMap &compiler_map,
                                          const std::string &ljf_tmpdir,
                                          const std::string &runtime_filename);
typedef int (*ljf_internal_start_entry_point_t)(ljf_main_t ljf_main,
                                                const std::string &language,
                                                const std::string &source_path,
                                                int argc, const char **argv);
} // namespace ljf

#pragma once

#include "ObjectHolder.hpp"
#include <ljf/ljf.hpp>
#include <ljf/runtime.hpp>

namespace llvm {
class Function;
} // namespace llvm

namespace ljf {

// TODO Write why Context and holding object pointer is required.
class Context {
private:
    class TemporaryHolders {
    private:
        std::vector<ObjectHolder> holders_;

    public:
        size_t add(IncrementedObjectPtrOrNativeValue &&obj) {
            auto index = holders_.size();
            holders_.push_back(std::move(obj));
            return index;
        }

        size_t add(Object *obj) {
            auto index = holders_.size();
            holders_.push_back(obj);
            return index;
        }
    };
    TemporaryHolders temporary_holders_;
    llvm::Module *LLVMModule_;
    Context *caller_context_ = nullptr;

public:
    explicit Context(llvm::Module *LLVMModule, Context *caller_context)
        : LLVMModule_(LLVMModule), caller_context_(caller_context) {}
    LJFHandle
    register_temporary_object(IncrementedObjectPtrOrNativeValue &&obj) {
        temporary_holders_.add(std::move(obj));
        return static_cast<LJFHandle>(obj);
    }

    LJFHandle register_temporary_object(Object *obj) {
        temporary_holders_.add(obj);
        return reinterpret_cast<LJFHandle>(obj);
    }

    // This function is prepared for future moving gc runtime.
    // Now this function does nothing.
    Object *get_from_handle(LJFHandle handle) {
        return reinterpret_cast<Object *>(handle);
    }
};

} // namespace ljf

namespace ljf::internal {

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

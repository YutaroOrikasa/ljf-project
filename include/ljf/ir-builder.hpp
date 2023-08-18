#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <string>
#include <variant>
#include <vector>

namespace ljf {

namespace detail {
    struct LLVMStuff {
        static inline llvm::LLVMContext llvmContext;
        static inline llvm::Type *LjfContextTy =
            llvm::Type::getVoidTy(llvmContext);
        static inline llvm::Type *LjfObjectTy =
            llvm::Type::getVoidTy(llvmContext);
        llvm::Module llvmModule;
        llvm::IRBuilder<> llvmIRBuilder;

        explicit LLVMStuff(const std::string &ModuleID)
            : llvmModule(ModuleID, llvmContext), llvmIRBuilder(llvmContext) {}
    };

} // namespace detail

class Module {
    detail::LLVMStuff llvmStuff_;

public:
    explicit Module(const std::string &ModuleID) : llvmStuff_(ModuleID) {}

    FunctionBuilder create_function() {
        std::vector<llvm::Type *> ArgsTy = {
            llvmStuff_.LjfContextTy, // LJF Context
            llvmStuff_.LjfObjectTy   // Environment and arguments object
        };
        llvm::FunctionType *FT =
            llvm::FunctionType::get(llvmStuff_.LjfObjectTy, ArgsTy, false);
        return;
    }
};

class ObjectRegister {};
class CStrLiteral {};

using Key = std::variant<CStrLiteral &, ObjectRegister &>;

enum class Visiblity { VISIBLE, HIDDEN };
enum class ConstantType { MUTABLE, MAYBE_CONSTANT };

struct Attribute {
    Visiblity visiblity = Visiblity::VISIBLE;
    ConstantType constant_type = ConstantType::MUTABLE;
};

class FunctionBuilder {
    detail::LLVMStuff *llvmStuff_;

public:
    explicit FunctionBuilder(detail::LLVMStuff &llvmStuff)
        : llvmStuff_(&llvmStuff) {}

    void begin_if() {}
    void create_else() {}
    void end_if() {}

    ObjectRegister create_new() {}
    ObjectRegister create_call(const ObjectRegister &function_object,
                               const ObjectRegister &arguments_object) {}
    template <typename Fn>
    ObjectRegister
    create_iteration(const ObjectRegister &iterable_object,
                     Fn /*  (FunctionBuilder &iter_body) -> void  */
                         &Iteration_body_builder) {}
    ObjectRegister create_get(const ObjectRegister &object, const Key &key) {}
    ObjectRegister create_set(const ObjectRegister &object, const Key &key,
                              const ObjectRegister &elem) {}
    ObjectRegister create_array_get(const ObjectRegister &object) {}
    ObjectRegister create_array_set(const ObjectRegister &object) {}
    ObjectRegister create_environment_get(const ObjectRegister &env,
                                          const Key &key) {}
    ObjectRegister create_environment_set(const ObjectRegister &env,
                                          const Key &key,
                                          const ObjectRegister &elem,
                                          bool deep_set = false) {}

    ObjectRegister create_ljf_undefined_object() {}
    ObjectRegister create_ljf_int64_object(int64_t value) {}
    ObjectRegister create_ljf_float_object(double value) {}

    ObjectRegister create_ljf_big_int_object(std::vector<int64_t> value) {}
    ObjectRegister create_ljf_int64_object_from_string(std::string value) {}
    ObjectRegister create_ljf_big_int_object_from_string(std::string value) {}

    /// @brief if value is represented in 64bit int, this function creates
    /// LJFInt64 object, otherwise creates LJFBigInt object.
    ObjectRegister create_ljf_integer_object_from_string(std::string value) {}

    ObjectRegister create_ljf_float_object_from_string(std::string value) {}
};

} // namespace ljf

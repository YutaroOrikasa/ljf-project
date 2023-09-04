#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <optional>
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

    template <typename Fn>
    void create_if(const ObjectRegister &cond, Fn &&then_body) {}

    template <typename Fn0, typename Fn1>
    void create_if_else(const ObjectRegister &cond, Fn0 &&then_body, Fn1 &&else_body) {}

    ObjectRegister create_new() {}
    ObjectRegister create_call(const ObjectRegister &function_object,
                               const ObjectRegister &arguments_object) {}

    /// @brief
    /// @tparam Fn (FunctionBuilder &Iteration_body_builder) -> void
    /// @param iterable_object
    /// @param Iteration_body
    /// @return
    template <typename Fn>
    ObjectRegister create_iteration(const ObjectRegister &iterable_object,
                                    Fn &&Iteration_body) {}

    /// @brief create cry-catch-finally
    /// @tparam FnTry (FunctionBuilder &try_body_builder) -> void
    /// @tparam FnCatch (FunctionBuilder &catch_body_builder) -> void
    /// @tparam FnFinally (FunctionBuilder &finally_body_builder) -> void
    /// @param try_body
    /// @param catch_body
    /// @param finally_body
    /// @return
    template <typename FnTry, typename FnCatch, typename FnFinally>
    ObjectRegister create_try_catch_finally(FnTry &&try_body,
                                            FnCatch &&catch_body,
                                            FnFinally &&finally_body) {}

    /// @brief create cry-catch-finally
    /// @tparam FnTry (FunctionBuilder &try_body_builder) -> void
    /// @tparam FnCatch (FunctionBuilder &catch_body_builder) -> void
    /// @tparam FnFinally (FunctionBuilder &finally_body_builder) -> void
    /// @param try_body
    /// @param catch_body
    /// @return
    template <typename FnTry, typename FnCatch>
    ObjectRegister create_try_catch(FnTry &&try_body, FnCatch &&catch_body) {}

    /// @brief create cry-catch-finally
    /// @tparam FnTry (FunctionBuilder &try_body_builder) -> void
    /// @tparam FnFinally (FunctionBuilder &finally_body_builder) -> void
    /// @param try_body
    /// @param finally_body
    /// @return
    template <typename FnTry, typename FnFinally>
    ObjectRegister create_try_finally(FnTry &&try_body,
                                      FnFinally &&finally_body) {}

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

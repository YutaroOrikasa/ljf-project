#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ljf::ir {

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

class FunctionBuilder;
class ObjectRegister {
    friend FunctionBuilder;
    llvm::Value *Value;
};
class CStrLiteral {};

using Key = std::variant<CStrLiteral &, ObjectRegister &>;

enum class Visiblity { VISIBLE, HIDDEN };
enum class ConstantType { MUTABLE, MAYBE_CONSTANT };

struct Attribute {
    Visiblity visiblity = Visiblity::VISIBLE;
    ConstantType constant_type = ConstantType::MUTABLE;
};

class FunctionBuilder {
    detail::LLVMStuff &llvmStuff_;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    llvm::Function *Func;

public:
    FunctionBuilder(detail::LLVMStuff &llvmStuff, std::string_view name)
        : llvmStuff_(llvmStuff),
          Builder(std::make_unique<llvm::IRBuilder<>>(llvmStuff_.llvmContext)) {
        std::vector<llvm::Type *> ArgsTy = {
            llvmStuff_.LjfContextTy, // LJF Context
            llvmStuff_.LjfObjectTy   // Environment and arguments object
        };
        llvm::FunctionType *FT =
            llvm::FunctionType::get(llvmStuff_.LjfObjectTy, ArgsTy, false);
        Func = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name,
                                      llvmStuff_.llvmModule);
        llvm::BasicBlock *BB =
            llvm::BasicBlock::Create(llvmStuff_.llvmContext, "", Func);
        Builder->SetInsertPoint(BB);
    }

    void create_ret(const ObjectRegister &ret_obj) {
        Builder->CreateRet(ret_obj.Value);
    }

    template <typename Fn>
    void create_if(const ObjectRegister &cond, Fn &&then_body) {
        throw "LJF Function builder: not implemented";
    }

    template <typename Fn0, typename Fn1>
    void create_if_else(const ObjectRegister &cond, Fn0 &&then_body,
                        Fn1 &&else_body) {
        throw "LJF Function builder: not implemented";
    }

    ObjectRegister create_new() {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_call(const ObjectRegister &function_object,
                               const ObjectRegister &arguments_object) {
        throw "LJF Function builder: not implemented";
    }

    /// @brief
    /// @tparam Fn (FunctionBuilder &Iteration_body_builder) -> void
    /// @param iterable_object
    /// @param Iteration_body
    /// @return
    template <typename Fn>
    ObjectRegister create_iteration(const ObjectRegister &iterable_object,
                                    Fn &&Iteration_body) {
        throw "LJF Function builder: not implemented";
    }

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
                                            FnFinally &&finally_body) {
        throw "LJF Function builder: not implemented";
    }

    /// @brief create cry-catch-finally
    /// @tparam FnTry (FunctionBuilder &try_body_builder) -> void
    /// @tparam FnCatch (FunctionBuilder &catch_body_builder) -> void
    /// @tparam FnFinally (FunctionBuilder &finally_body_builder) -> void
    /// @param try_body
    /// @param catch_body
    /// @return
    template <typename FnTry, typename FnCatch>
    ObjectRegister create_try_catch(FnTry &&try_body, FnCatch &&catch_body) {
        throw "LJF Function builder: not implemented";
    }

    /// @brief create cry-catch-finally
    /// @tparam FnTry (FunctionBuilder &try_body_builder) -> void
    /// @tparam FnFinally (FunctionBuilder &finally_body_builder) -> void
    /// @param try_body
    /// @param finally_body
    /// @return
    template <typename FnTry, typename FnFinally>
    ObjectRegister create_try_finally(FnTry &&try_body,
                                      FnFinally &&finally_body) {
        throw "LJF Function builder: not implemented";
    }

    ObjectRegister create_get(const ObjectRegister &object, const Key &key) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_set(const ObjectRegister &object, const Key &key,
                              const ObjectRegister &elem) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_array_get(const ObjectRegister &object) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_array_set(const ObjectRegister &object) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_environment_get(const ObjectRegister &env,
                                          const Key &key) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_environment_set(const ObjectRegister &env,
                                          const Key &key,
                                          const ObjectRegister &elem,
                                          bool deep_set = false) {
        throw "LJF Function builder: not implemented";
    }

    ObjectRegister create_ljf_undefined_object() {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_ljf_int64_object(int64_t value) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_ljf_float_object(double value) {
        throw "LJF Function builder: not implemented";
    }

    ObjectRegister create_ljf_big_int_object(std::vector<int64_t> value) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_ljf_int64_object_from_string(std::string value) {
        throw "LJF Function builder: not implemented";
    }
    ObjectRegister create_ljf_big_int_object_from_string(std::string value) {
        throw "LJF Function builder: not implemented";
    }

    /// @brief if value is represented in 64bit int, this function creates
    /// LJFInt64 object, otherwise creates LJFBigInt object.
    ObjectRegister create_ljf_integer_object_from_string(std::string value) {
        throw "LJF Function builder: not implemented";
    }

    ObjectRegister create_ljf_float_object_from_string(std::string value) {
        throw "LJF Function builder: not implemented";
    }
};

class Module {
    detail::LLVMStuff llvmStuff_;

public:
    explicit Module(const std::string &ModuleID) : llvmStuff_(ModuleID) {}

    FunctionBuilder create_function() {
        throw "LJF Function builder: not implemented";
    }

    FunctionBuilder create_module_main_function() {
        // LJF module main function's signature:
        // LJFHandle ljf_module_main(ljf::Context *, LJFHandle env);
        // Returned object is module object.

        return FunctionBuilder(llvmStuff_, "ljf_module_main");
    }
};

} // namespace ljf::ir

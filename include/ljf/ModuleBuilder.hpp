#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace ljf {

/// This is not stable api, be careful to use.
struct Underlying {
    llvm::LLVMContext llvmContext_;
    llvm::Module llvmModule_;
    llvm::IRBuilder<> llvmIRBuilder_;
};

class FunctionBuilder {};

class ModuleBuilder {
    Underlying underlying_;

public:
    ModuleBuilder() : {}

    FunctionBuilder create_function() {}
};
} // namespace ljf

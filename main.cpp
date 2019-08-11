// loading llvm bitcode from given filename.

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/SmallString.h>

#include <gperftools/profiler.h>

#include <iostream>
#include <map>

#include <cstdlib>
#include <dlfcn.h>

#include <ljf/runtime.hpp>
#include <ljf/ljf.hpp>

using namespace ljf;

using namespace std::literals::string_literals;

class LLVMAsmCompiler : public ljf::Compiler
{
private:
    llvm::LLVMContext llvm_context;
    llvm::SMDiagnostic err;

public:
    std::unique_ptr<llvm::Module> compile(const std::string &source_path) override
    {
        auto module = llvm::parseAssemblyFile(source_path, err, llvm_context);
        if (!module)
        {
            err.print(source_path.c_str(), llvm::errs());
        }

        return module;
    }
};

int main(int argc, const char **argv)
{
    std::vector<std::string> arg(argv, argv + argc);
    assert(arg.size() >= 2);

    auto ljf_runtime_path = "build/runtime.so"s;
    auto input_ll_file = arg[1];

    ljf::CompilerMap compiler_map {
        {"llvm asm", std::make_shared<LLVMAsmCompiler>()},
    };

    ljf::initialize(compiler_map, "./tmp", ljf_runtime_path);

    return ljf::start_entry_point_of_source("llvm asm", input_ll_file, argc, argv);
}

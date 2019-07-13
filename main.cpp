// loading llvm bitcode from given filename.

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/ADT/SmallString.h>

#include <iostream>

#include <cstdlib>
#include <dlfcn.h>

#include "ljf/runtime.hpp"

ljf::Environment* ljf_internal_new_environment();

struct SmallString : llvm::SmallString<32> {
    SmallString() = default;
    // using llvm::SmallString<32>::SmallString;

    /* implicit */ SmallString(const llvm::Twine& tw) {
        tw.toVector(*this);
    }
};

int main()
{
    auto ljf_runtime_path = SmallString("build/runtime.so");
    auto input_ll_file = SmallString("build/llcode/fibo.cpp.ll");
    llvm::LLVMContext context;
    llvm::SMDiagnostic err;
    auto [module, index] = llvm::parseAssemblyFileWithIndex(input_ll_file, err, context);
    if (!module)
    {
        llvm::errs() << "module null\n";
        err.print("main", llvm::errs());
        return 1;
    }

    for (auto &func : module->functions())
    {
        llvm::errs() << "*** begin editing"
                     << "\n";
        const auto name = func.getName();

        if (name == "main")
        {
            // skip main()
            llvm::errs() << "*** skip " << name << ": "
                         << *func.getFunctionType()
                         << "\n";
            continue;
        }

        if (name == "module_main")
        {
            // skip module_main()
            llvm::errs() << "*** skip " << name << ": "
                         << *func.getFunctionType()
                         << "\n";
            continue;
        }

        if (name == "_Z11module_main9AnyObject")
        {
            // skip _Z11module_main9AnyObject()
            llvm::errs() << "*** skip " << name << ": "
                         << *func.getFunctionType()
                         << "\n";
            continue;
        }

        if (func.isDeclaration())
        {
            // skip declare ty @func(ty...)
            llvm::errs() << "*** skip declaration of " << name << ": "
                         << *func.getFunctionType()
                         << "\n";
            continue;
        }

        llvm::errs() << "editing " << name << ": ";

        llvm::errs() << *func.getFunctionType() << "\n";

        // func.setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

        llvm::errs() << "*** end editing"
                     << "\n";
    }

    // auto input_ll_file_path_obj = std::filesystem::path(input_ll_file);

    // auto output_bc_file = input_ll_file_path_obj.stem().string() + ".bc";

    SmallString output_bc_path = "./tmp/" + input_ll_file;
    llvm::sys::path::replace_extension(output_bc_path, "bc");
    auto output_bc_dirname = llvm::sys::path::parent_path(output_bc_path);

    if (auto err_code = llvm::sys::fs::create_directories(output_bc_dirname,
                                          /* IgnoreExisting */ true,
                                          llvm::sys::fs::perms::owner_all))
    {
        std::cerr << "create_directory(" << output_bc_dirname.str() << ") failed with error code " << err_code << "\n";
        exit(1);
    }

    {
        std::error_code EC;
        llvm::ToolOutputFile out{output_bc_path, EC, llvm::sys::fs::OF_None};
        if (EC)
        {
            llvm::errs() << "llvm::ToolOutputFile ctor: " << EC.message() << '\n';
            exit(1);
        }
        out.keep();

        llvm::WriteBitcodeToFile(*module, out.os());

        // out is flushed and closed by ~ToolOutputFile.
    }


    // compile
    SmallString output_so_path = "./tmp/" + input_ll_file;

    llvm::sys::path::replace_extension(output_so_path, "so");


    auto compile_command_line = "clang++ -L/usr/local/opt/llvm/lib -lLLVM "
        + output_bc_path + " " + ljf_runtime_path + " -shared -o " + output_so_path;
    llvm::errs() << compile_command_line << '\n';
    if (auto e = std::system(compile_command_line.str().c_str())) {
        llvm::errs() << "compile failed: " << e << '\n';
        exit(1);
    }

    // load
    auto handle = dlopen(output_so_path.c_str(), RTLD_LAZY);
    if (!handle) {
        llvm::errs() << "load failed (dlopen): " << dlerror() << '\n';
        exit(1);
    }
    llvm::errs() << "HOGE\n";

    auto symbol = dlsym(handle, "module_main");
    if (!symbol) {
        llvm::errs() << "load failed (dlsym): " << dlerror() << '\n';
        exit(1);
    }

    auto module_main_fptr = reinterpret_cast<LJFObject* (*)(LJFObject*)>(symbol);
    LJFObject* arg = ljf_new_object_with_native_data(10);
    auto env = ljf_internal_new_environment();
    ljf_set_object_to_environment(env, "n", arg);
    try {
        LJFObject* ret = module_main_fptr(env);
        if (!ret) {
            std::cerr << "result: " << "undefined" << std::endl;
            return 1;
        }
        auto val = ljf_get_native_data(ret);
        std::cerr << "result: " << val << std::endl;
    } catch (const std::exception& e) {
        llvm::errs() << "module_main() exception: " << e.what() << "\n";
        exit(1);
    } catch (...) {
        llvm::errs() << "module_main() exception: unknown" << "\n";
        exit(1);
    }
    return 0;
}

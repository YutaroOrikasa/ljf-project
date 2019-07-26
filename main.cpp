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

#include <google/profiler.h>

#include <iostream>
#include <map>

#include <cstdlib>
#include <dlfcn.h>

#include <ljf/runtime.hpp>
#include "./runtime-internal.hpp"

using namespace ljf;

struct SmallString : llvm::SmallString<32>
{
    SmallString() = default;
    // using llvm::SmallString<32>::SmallString;

    /* implicit */ SmallString(const llvm::Twine &tw)
    {
        tw.toVector(*this);
    }
};

int main(int argc, const char **argv)
{
    std::vector<std::string> arg(argv, argv + argc);
    assert(arg.size() >= 2);
    if (arg.size() == 2)
    {
        arg.push_back("");
    }

    auto clang_opt = arg[2];

    std::cerr << "clang_opt: " << clang_opt << "\n";

    std::map<LJFFunctionId, llvm::Function *> func_to_register;
    ObjectHolder module_func_table = ljf_new_object();
    auto ljf_runtime_path = SmallString("build/runtime.so");
    auto input_ll_file = SmallString(arg[1]);
    llvm::LLVMContext llvm_context;
    llvm::SMDiagnostic err;
    llvm::IRBuilder ir_builder{llvm_context};
    auto [module, index] = llvm::parseAssemblyFileWithIndex(input_ll_file, err, llvm_context);
    if (!module)
    {
        llvm::errs() << "module null\n";
        err.print("main", llvm::errs());
        return 1;
    }

    for (auto &func : module->functions())
    {
        llvm::errs() << "*** begin registering function"
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

        if (func.isDeclaration())
        {
            // skip declare ty @func(ty...)
            llvm::errs() << "*** skip declaration of " << name << ": "
                         << *func.getFunctionType()
                         << "\n";
            continue;
        }

        llvm::errs() << "registering " << name << ": ";

        auto id = ljf::register_llvm_function(&func);
        func_to_register[id] = &func;
        ljf_set_function_id_to_function_table(module_func_table.get(), func.getName().data(), id);

        // func.setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

        llvm::errs() << "*** end registering function"
                     << "\n";
    } // end for

    {
        auto void_ty = llvm::Type::getVoidTy(llvm_context);
        auto i8_ty = llvm::Type::getInt8Ty(llvm_context);
        auto i8_ptr_ty = llvm::PointerType::get(i8_ty, 0);
        // auto ljf_object_ty = llvm::StructType::create(llvm_context, "LJFObject");
        // auto ljf_object_ptr_ty = llvm::PointerType::get(ljf_object_ty, 0);
        auto ljf_module_init_fn_ty = llvm::FunctionType::get(void_ty, {});
        auto ljf_module_init_fn = llvm::Function::Create(ljf_module_init_fn_ty, llvm::Function::ExternalLinkage, "ljf_module_init", *module);

        auto i64_ty = llvm::Type::getInt64Ty(llvm_context);
        auto ljf_internal_set_native_function_ty = llvm::FunctionType::get(void_ty, {i64_ty, i8_ptr_ty}, false);
        auto ljf_internal_set_native_function = llvm::Function::Create(ljf_internal_set_native_function_ty, llvm::Function::ExternalLinkage, "ljf_internal_set_native_function", *module);

        auto bb = llvm::BasicBlock::Create(llvm_context, "entry", ljf_module_init_fn);
        ir_builder.SetInsertPoint(bb);

        for (const auto &[id, fn] : func_to_register)
        {
            auto id_const = llvm::ConstantInt::get(i64_ty, id);
            auto casted_fn_ptr = ir_builder.CreateBitCast(fn, i8_ptr_ty);
            ir_builder.CreateCall(ljf_internal_set_native_function, {id_const, casted_fn_ptr});
        }
        ir_builder.CreateRetVoid();
    }

    {
        // dump
        std::error_code EC;
        llvm::ToolOutputFile out{"./_dump.ll", EC, llvm::sys::fs::OF_None};
        if (EC)
        {
            llvm::errs() << "llvm::ToolOutputFile ctor: " << EC.message() << '\n';
            exit(1);
        }

        out.os() << *module;
        out.keep();

        // out is flushed and closed by ~ToolOutputFile.
    }

    assert(!llvm::verifyModule(*module, &llvm::errs()) && "invalid llvm module generated");

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

    SmallString compile_command_line = "clang++ " + clang_opt + " -L/usr/local/opt/llvm/lib -lLLVM " + output_bc_path + " " + ljf_runtime_path + " -shared -o " + output_so_path;
    llvm::errs() << compile_command_line << '\n';
    if (auto e = std::system(compile_command_line.c_str()))
    {
        llvm::errs() << "compile failed: " << e << '\n';
        exit(1);
    }

    // load
    auto handle = dlopen(output_so_path.c_str(), RTLD_LAZY);
    if (!handle)
    {
        llvm::errs() << "load failed (dlopen): " << dlerror() << '\n';
        exit(1);
    }
    llvm::errs() << "HOGE\n";

    auto ljf_module_init_addr = dlsym(handle, "ljf_module_init");
    if (!ljf_module_init_addr)
    {
        llvm::errs() << "load failed (dlsym): " << dlerror() << '\n';
        exit(1);
    }
    reinterpret_cast<void (*)()>(ljf_module_init_addr)();

    auto addr = dlsym(handle, "module_main");
    if (!addr)
    {
        llvm::errs() << "load failed (dlsym): " << dlerror() << '\n';
        exit(1);
    }

    auto module_main_fptr = reinterpret_cast<LJFObject *(*)(LJFObject *, LJFObject *)>(addr);
    auto env_holder = ljf::internal::create_environment();
    auto env = env_holder.get();
    try
    {
        // ProfilerStart("tmp/main.prof");
        LJFObject *ret = module_main_fptr(env, module_func_table.get());
        // ProfilerStop();
        if (!ret)
        {
            std::cerr << "result: "
                      << "undefined" << std::endl;
            return 1;
        }
        auto val = ljf_get_native_data(ret);
        std::cerr << "result: " << val << std::endl;
    }
    catch (const std::exception &e)
    {
        llvm::errs() << "module_main() exception: " << e.what() << "\n";
        exit(1);
    }
    catch (...)
    {
        llvm::errs() << "module_main() exception: unknown"
                     << "\n";
        exit(1);
    }
    return 0;
}

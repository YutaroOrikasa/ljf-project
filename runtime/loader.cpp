#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>

#include <dlfcn.h>

#include <stdexcept>
#include <string>
#include <iostream>

#include "ljf/ljf.hpp"
#include <ljf/runtime.hpp>
#include "runtime-internal.hpp"

using namespace std::literals::string_literals;

namespace
{
struct SmallString : llvm::SmallString<32>
{
    SmallString() = default;
    // using llvm::SmallString<32>::SmallString;

    /* implicit */ SmallString(const llvm::Twine &tw)
    {
        tw.toVector(*this);
    }
};
} // namespace

namespace ljf
{
namespace
{
llvm::LLVMContext llvm_context;

struct Context
{
    CompilerMap compiler_map;
    std::string ljf_tmpdir;
    std::string ljf_runtime_filename;
};
// set by ljf::initialize()
std::unique_ptr<Context> context = nullptr;

void check_context_initialized()
{
    if (!context)
    {
        throw std::logic_error("ljf::initialize() is not called");
    }
}

bool verbose = false;
auto &verbs()
{
    if (verbose)
    {
        return llvm::errs();
    }
    else
    {
        return llvm::nulls();
    }
}
} // namespace

void initialize(const CompilerMap &compiler_map, const std::string &ljf_tmpdir, const std::string &runtime_filename)
{
    if (context)
    {
        throw std::logic_error("ljf::initialize() is called twice or more");
    }

    context = std::make_unique<Context>(Context{compiler_map, ljf_tmpdir, runtime_filename});

    // remove ljf_tmpdir
    if (auto err_code = llvm::sys::fs::remove_directories(context->ljf_tmpdir))
    {
        throw std::system_error(err_code, "remove ljf tmpdir \"" + context->ljf_tmpdir + "\" failed");
    }

    // create ljf_tmpdir
    // for security,
    //      - fail if ljf_tmpdir exists (ljf_tmpdir should be removed above)
    //      - permission is 700
    if (auto err_code = llvm::sys::fs::create_directories(context->ljf_tmpdir,
                                                          /* IgnoreExisting */ false,
                                                          llvm::sys::fs::perms::owner_all))
    {
        throw std::system_error(err_code, "create ljf tmpdir \"" + context->ljf_tmpdir + "\" failed");
    }
}

int start_entry_point_of_bitcode(const std::string &bitcode_path, int argc, const char **argv);

} // namespace ljf

namespace ljf::internal
{

/// return: environment of module
ObjectHolder load_source_code(const std::string &language, const std::string &source_path, Object *env)
{

    check_context_initialized();
    if (!context->compiler_map.count(language))
    {
        throw std::invalid_argument("No such compiler for `" + language + "`");
    }
    std::unique_ptr<llvm::Module> module = context->compiler_map.at(language)->compile(source_path);
    if (!module)
    {
        throw std::runtime_error("compiling of `" + language + "` code failed");
    }

    if (llvm::verifyModule(*module, &llvm::errs()))
    {
        throw ljf::runtime_error("compiler of `" + language + "` generated invalid llvm module; source: " + source_path);
    }

    ObjectHolder module_func_table = ljf_new_object();
    std::map<LJFFunctionId, llvm::Function *> func_to_register;

    for (auto &func : module->functions())
    {
        verbs() << "*** begin registering function"
                << "\n";
        const auto name = func.getName();

        if (name == "module_main")
        {
            // skip module_main()
            verbs() << "*** skip " << name << ": "
                    << *func.getFunctionType()
                    << "\n";
            continue;
        }

        if (func.isDeclaration())
        {
            // skip declare ty @func(ty...)
            verbs() << "*** skip declaration of " << name << ": "
                    << *func.getFunctionType()
                    << "\n";
            continue;
        }

        verbs() << "registering " << name << ": ";

        auto id = ljf_internal_register_llvm_function(&func);
        func_to_register[id] = &func;
        ljf_set_function_id_to_function_table(module_func_table.get(), func.getName().data(), id);

        // func.setLinkage(llvm::GlobalValue::LinkageTypes::InternalLinkage);

        verbs() << "*** end registering function"
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

        llvm::IRBuilder ir_builder{llvm_context};

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
        llvm::raw_fd_ostream out{context->ljf_tmpdir + "/_dump.ll", EC};
        if (EC)
        {
            llvm::errs() << "llvm::raw_fd_ostream ctor: " << EC.message() << '\n';
            exit(1);
        }

        out << *module;
    }

    std::string output_bc_dir = context->ljf_tmpdir + "/" + module->getModuleIdentifier();
    if (auto err_code = llvm::sys::fs::create_directories(output_bc_dir))
    {
        throw std::system_error(err_code);
    }

    SmallString output_bc_path;
    if (auto err_code = llvm::sys::fs::createUniqueFile(output_bc_dir + "/ljf-%%-%%-%%-%%.bc", output_bc_path))
    {
        throw std::system_error(err_code);
    }

    {
        std::error_code EC;
        llvm::raw_fd_ostream out{output_bc_path, EC};
        if (EC)
        {
            throw std::system_error(EC);
        }

        llvm::WriteBitcodeToFile(*module, out);
    }

    // compile
    SmallString output_so_path = output_bc_path;

    llvm::sys::path::replace_extension(output_so_path, "so");

    SmallString compile_command_line = "clang++ -L/usr/local/opt/llvm/lib -lLLVM " + output_bc_path + " " + context->ljf_runtime_filename + " -shared -o " + output_so_path;
    llvm::errs() << compile_command_line << '\n';
    if (auto e = std::system(compile_command_line.c_str()))
    {
        throw ljf::runtime_error("compile failed: exited with " + std::to_string(e) + ", command line: " + compile_command_line.c_str());
    }

    auto module_handle = dlopen(output_so_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!module_handle)
    {
        throw std::runtime_error("dlopen failed: "s + dlerror());
    }
    auto module_main_addr = dlsym(module_handle, "module_main");
    if (!module_main_addr)
    {
        throw std::runtime_error("dlsym failed: "s + dlerror());
    }

    auto ljf_module_init_addr = dlsym(module_handle, "ljf_module_init");
    if (!ljf_module_init_addr)
    {
        throw std::runtime_error("dlsym failed: "s + dlerror());
    }

    auto ljf_module_init = reinterpret_cast<void (*)()>(ljf_module_init_addr);
    ljf_module_init();

    auto module_main = reinterpret_cast<LJFObject *(*)(LJFObject *, LJFObject *)>(module_main_addr);

    ObjectHolder ret = module_main(env, module_func_table.get());
    return ret;
}

} // namespace ljf::internal

using namespace ljf;

// ljf_internal
extern "C"
{
    void ljf_internal_initialize(const CompilerMap &compiler_map, const std::string &ljf_tmpdir, const std::string &runtime_filename)
    {
        ljf::initialize(compiler_map, ljf_tmpdir, runtime_filename);
    }
} // extern "C"

namespace ljf::internal::check_
{
ljf_internal_initialize_t ljf_internal_initialize_ = ljf_internal_initialize;
} // namespace ljf::internal::check_

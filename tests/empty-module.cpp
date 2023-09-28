
#include <ljf/ljf.hpp>
#include <ljf/runtime.hpp>

using namespace ljf;

using namespace std::literals::string_literals;

class EmptyModuleCompiler : public ljf::Importer {
private:
    llvm::LLVMContext llvm_context;
    llvm::SMDiagnostic err;

public:
    std::unique_ptr<llvm::Module>
    compile(const std::string &source_path) override {
        auto module = llvm::parseAssemblyFile(source_path, err, llvm_context);
        if (!module) {
            err.print(source_path.c_str(), llvm::errs());
        }

        return module;
    }
};

int main(int argc, const char **argv) {
    std::vector<std::string> arg(argv, argv + argc);
    assert(arg.size() >= 2);

    auto ljf_runtime_path = "build/runtime.so"s;
    auto input_ll_file = arg[1];

    ljf::ImporterMap compiler_map{
        {"llvm asm", std::make_shared<LLVMAsmCompiler>()},
    };

    ljf::initialize(compiler_map, "./tmp/ljf", ljf_runtime_path);

    return ljf::start_entry_point_of_source("llvm asm", input_ll_file, argc,
                                            argv);
}

#pragma once

#include "ljf/IRCompiler.hpp"
#include "ljf/IRCompilerBase.hpp"

#include <llvm/IR/Module.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace ljf {
// class LoadError : std::runtime_error {

// };

class Compiler {
public:
    Compiler() = default;
    virtual ~Compiler() = default;

    virtual std::unique_ptr<llvm::Module>
    compile(const std::string &source_file_path) = 0;
};

using CompilerMap = std::unordered_map<std::string, std::shared_ptr<Compiler>>;

void initialize(
    const CompilerMap &compiler_map, const std::string &ljf_tmpdir,
    const std::string &runtime_filename = "",
    std::unique_ptr<IRCompilerBase> = std::make_unique<IRCompiler>());

typedef int (*ljf_main_t)(int argc, const char **argv);
int start_entry_point_of_function_ptr(ljf_main_t ljf_main, int argc,
                                      const char **argv);

int start_entry_point_of_native_dynamic_library(
    std::string dynamic_library_path, int argc, const char **argv);

int start_entry_point_of_source(const std::string &language,
                                const std::string &source_path, int argc,
                                const char **argv);
} // namespace ljf

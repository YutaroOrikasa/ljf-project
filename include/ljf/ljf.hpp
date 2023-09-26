#pragma once

#include "ljf/IRCompiler.hpp"
#include "ljf/IRCompilerBase.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/ir-builder.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace ljf {
// class LoadError : std::runtime_error {

// };

class Importer {
public:
    Importer() = default;
    virtual ~Importer() = default;

    virtual LJFHandle import(ljf::Context *ctx, LJFHandle env,
                             const std::string &source_file_path) = 0;
};

using ImporterMap = std::unordered_map<std::string, std::shared_ptr<Importer>>;

void initialize(
    const ImporterMap &importer_map, const std::string &ljf_tmpdir,
    const std::string &runtime_filename = "",
    std::unique_ptr<IRCompilerBase> = std::make_unique<IRCompiler>());

ljf::ObjectWrapper import_main_module(const std::string &src_path,
                                      const std::string &language);

typedef int (*ljf_main_t)(int argc, const char **argv);
int start_entry_point_of_function_ptr(ljf_main_t ljf_main, int argc,
                                      const char **argv);

int start_entry_point_of_native_dynamic_library(
    std::string dynamic_library_path, int argc, const char **argv);

int start_entry_point_of_source(const std::string &language,
                                const std::string &source_path, int argc,
                                const char **argv);
} // namespace ljf

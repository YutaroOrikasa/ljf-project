#pragma once

#include "ljf/IRCompilerBase.hpp"

#include <filesystem>

namespace ljf {
class Module;
class IRCompiler : public IRCompilerBase {

public:
    /// @brief
    /// @param basedir
    /// @param
    /// @return Path to compiled .so file
    virtual std::filesystem::path compile(const std::filesystem::path &basedir,
                                          Module &module) override {
        throw "stab";
    }
};
} // namespace ljf

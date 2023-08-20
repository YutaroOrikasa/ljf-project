#pragma once

#include <filesystem>

namespace ljf {
class Module;
class IRCompilerBase {

public:
    /// @brief
    /// @param basedir
    /// @param
    /// @return Path to compiled .so file
    virtual std::filesystem::path compile(const std::filesystem::path &basedir, Module &);
};
} // namespace ljf

#pragma once

#include <string>

namespace ljf::python::literal
{
class StringLiteral
{
private:
    std::string prefix_;
    std::string contents_;

public:
    StringLiteral(std::string prefix, std::string contents)
        : prefix_(prefix),
          contents_(contents) {}
    
    const std::string& prefix() const noexcept
    {
        return prefix_;
    }
    const std::string& contents() const noexcept
    {
        return contents_;
    }
};
} // namespace ljf::python::literal

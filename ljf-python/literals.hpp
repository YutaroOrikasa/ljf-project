#pragma once

#include <string>
#include <algorithm>

namespace ljf::python::literals
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

    const std::string &prefix() const noexcept
    {
        return prefix_;
    }
    const std::string &contents() const noexcept
    {
        return contents_;
    }
};

class IntegerLiteral
{
private:
    size_t radix_ = 0;
    std::string integer_str_;

public:
    IntegerLiteral(size_t radix, const std::string &integer_str)
        : radix_(radix), integer_str_(integer_str)
    {
        auto &s = integer_str_;
        // remove-erase idiom
        // Remove separator char '_'.
        s.erase(std::remove(s.begin(), s.end(), '_'), s.end());

        if (radix != 10)
        {
            assert(radix == 2 || radix == 8 || radix == 16);
            // remove prefix such like '0x'
            assert(s.size() > 2);
            s.erase(0, 2);
        }
    }

    size_t radix() const noexcept
    {
        return radix_;
    }

    /// The result does not contain prefix such like '0x'.
    const std::string& integer_str() const noexcept
    {
        return integer_str_;
    }
};
} // namespace ljf::python::literals

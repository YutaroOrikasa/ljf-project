#pragma once

#include <string>

namespace ljf::python
{

struct zero_based_index_t
{
};
inline constexpr zero_based_index_t zero_based_index;

struct one_based_index_t
{
};
inline constexpr one_based_index_t one_based_index;

class SourceLocation
{
    std::string source_file_name_;
    // row_ and col_ start at 1
    size_t row_ = -1; // set -1 as uninitialized value
    size_t col_ = -1; // set -1 as uninitialized value

public:
    explicit SourceLocation(one_based_index_t,
                            const std::string &source_file_name,
                            size_t row,
                            size_t col)
        : source_file_name_(source_file_name),
          row_(row),
          col_(col) {}

    explicit SourceLocation(zero_based_index_t,
                            const std::string &source_file_name,
                            size_t row,
                            size_t col)
        : source_file_name_(source_file_name),
          row_(row + 1),
          col_(col + 1) {}

    const std::string &source_file_name() const noexcept
    {
        return source_file_name_;
    }

    /// returned value is one-based index
    size_t row() const noexcept
    {
        return row_;
    }

    /// returned value is one-based index
    size_t column() const noexcept
    {
        return col_;
    }

    template <typename Out>
    friend Out &operator<<(Out &out, const SourceLocation &loc)
    {
        return out << loc.source_file_name() << ":" << loc.row() << ":" << loc.column();
    }
};
} // namespace ljf::python
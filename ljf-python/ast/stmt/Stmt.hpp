#pragma once

#include <variant>
#include <memory>
#include <type_traits>

namespace ljf::python::ast
{

struct StmtVariant;

class Stmt
{
private:
    std::unique_ptr<StmtVariant> stmt_var_ptr_;

public:
    // Check is_stmt_impl member type
    // so that static_assert that use std::is_constructible works.
    template <typename T,
              typename = typename std::remove_reference_t<T>::is_stmt_impl>
    /*implicit*/ Stmt(T &&t)
    {
        stmt_var_ptr_ = std::make_unique<StmtVariant>(std::forward<T>(t));
    }

    Stmt(const Stmt &other)
    {
        stmt_var_ptr_ = std::make_unique<StmtVariant>(*other.stmt_var_ptr_);
    }

    Stmt(Stmt &&) = default;

    Stmt &operator=(const Stmt &other)
    {
        return *this = Stmt(other);
    }

    Stmt &operator=(Stmt &&) = default;

    template <typename Visitor>
    auto accept(Visitor &&visitor) const
    {
        assert(stmt_var_ptr_);
        return std::visit(std::forward<Visitor>(visitor), *stmt_var_ptr_);
    }

    const StmtVariant& stmt_variant() const
    {
        assert(stmt_var_ptr_);
        return *stmt_var_ptr_;
    }
};

static_assert(std::is_copy_constructible_v<Stmt>);

namespace detail
{

} // namespace detail
} // namespace ljf::python::ast

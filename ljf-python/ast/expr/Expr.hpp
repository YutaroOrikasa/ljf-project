#pragma once

namespace ljf::python::ast
{

struct ExprVariant;

class Expr
{
private:
    std::unique_ptr<ExprVariant> expr_var_ptr_;

public:
    // Check is_expr_impl member type
    // so that static_assert that use std::is_constructible works.
    template <typename T,
              typename = typename std::remove_reference_t<T>::is_expr_impl>
    /*implicit*/ Expr(T &&t)
    {
        expr_var_ptr_ = std::make_unique<ExprVariant>(std::forward<T>(t));
    }

    Expr(const Expr &other)
    {
        expr_var_ptr_ = std::make_unique<ExprVariant>(*other.expr_var_ptr_);
    }

    Expr(Expr &&) = default;

    Expr &operator=(const Expr &other)
    {
        return *this = Expr(other);
    }

    Expr &operator=(Expr &&) = default;

    template <typename Visitor>
    auto accept(Visitor &&visitor) const
    {
        assert(expr_var_ptr_);
        return std::visit(std::forward<Visitor>(visitor), *expr_var_ptr_);
    }

    const ExprVariant &expr_variant() const
    {
        assert(expr_var_ptr_);
        return *expr_var_ptr_;
    }
};

static_assert(std::is_copy_constructible_v<Expr>);

namespace detail
{
class SingleTokenExpr
{
private:
    Token token_;

public:
    SingleTokenExpr(Token &&token) : token_(std::move(token)) {}

    const Token &token() const noexcept
    {
        return token_;
    }
};

} // namespace detail
} // namespace ljf::python::ast

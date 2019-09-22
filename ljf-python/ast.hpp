#pragma once

#include <tuple>
#include <variant>
#include <memory>
#include <type_traits>

#include "Token.hpp"

namespace ljf::python::ast
{
struct ExprVariant;

class Expr
{
private:
    std::unique_ptr<ExprVariant> expr_var_ptr_;

public:
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
};

static_assert(std::is_copy_constructible_v<Expr>);

class StringLiteralExpr
{
private:
    Token token_;

public:
    using is_expr_impl = void;
    StringLiteralExpr(Token &&token) : token_(std::move(token)) {}

    const Token &token() const noexcept
    {
        return token_;
    }
};

class IntegerLiteralExpr
{
private:
    Token token_;

public:
    using is_expr_impl = void;
    IntegerLiteralExpr(Token &&token) : token_(std::move(token)) {}

    const Token &token() const noexcept
    {
        return token_;
    }
};

namespace detail
{
class SingleTokenExpr
{
private:
    Token token_;

public:
    using is_expr_impl = void;
    SingleTokenExpr(Token &&token) : token_(std::move(token)) {}

    const Token &token() const noexcept
    {
        return token_;
    }
};

class EnclosureExpr
{
private:
    /* data */
public:
    using is_expr_impl = void;
    EnclosureExpr(std::tuple<>) {}
};

} // namespace detail

struct IdentifierExpr : detail::SingleTokenExpr
{
    using SingleTokenExpr::SingleTokenExpr;
};

class ListExpr
{
private:
public:
    using is_expr_impl = void;
    ListExpr(std::tuple<>) {}
};

/// kind of
///  1. (expr)
///  2. ()
class ParenthFormExpr
{
private:
    /* data */
public:
    using is_expr_impl = void;
    ParenthFormExpr() {}
};

struct DictExpr : detail::EnclosureExpr
{
    using EnclosureExpr::EnclosureExpr;
};

struct UnaryExpr
{
    using is_expr_impl = void;
    Token operator_;
    Expr operand_;

    UnaryExpr(Token _operator, Expr operand)
        : operator_(std::move(_operator)),
          operand_(std::move(operand)) {}
};

struct BinaryExpr
{
    using is_expr_impl = void;
    Token operator_;
    Expr left_;
    Expr right_;

    BinaryExpr(Expr left, Token oper, Expr right)
        : operator_(std::move(oper)),
          left_(std::move(left)),
          right_(std::move(right)) {}
};

struct ConditionalExpr
{
private:
    struct IfElse
    {
        Expr if_;
        Expr else_;
    };

public:
    using is_expr_impl = void;
    Expr or_test_;
    std::optional<IfElse> if_else_;

    ConditionalExpr(Expr or_test, 
    std::optional<std::tuple<Expr, Expr>> opt_if_else)
    : or_test_(std::move(or_test))
    {
        if (opt_if_else)
        {
            auto [if_, else_] = std::move(*opt_if_else);
            if_else_ = {std::move(if_), std::move(else_)};
        }
    }
};

struct ExprVariant : std::variant<
                         StringLiteralExpr,
                         IntegerLiteralExpr,
                         IdentifierExpr,
                         ListExpr,
                         ParenthFormExpr,
                         DictExpr,
                         UnaryExpr,
                         BinaryExpr,
                         ConditionalExpr>
{
    using variant::variant;
};

} // namespace ljf::python::ast

#pragma once

#include <tuple>
#include <variant>
#include <memory>

#include "Token.hpp"

namespace ljf::python::ast
{

class StringLiteralExpr
{
private:
    Token token_;

public:
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
    ParenthFormExpr(std::tuple<>) {}
};

struct DictExpr : detail::EnclosureExpr
{
    using EnclosureExpr::EnclosureExpr;
};

using ExprVariant = std::variant<
    StringLiteralExpr,
    IntegerLiteralExpr,
    IdentifierExpr,
    ListExpr,
    ParenthFormExpr,
    DictExpr>;

class Expr
{
private:
    std::unique_ptr<ExprVariant> expr_var_ptr_;
public:
    template<typename T>
    Expr(T&&t) {
        expr_var_ptr_ = std::make_unique<ExprVariant>(std::forward<T>(t));
    }

    template <typename Visitor>
    auto accept(Visitor && visitor) const
    {
        assert(expr_var_ptr_);
        return std::visit(std::forward<Visitor>(visitor), *expr_var_ptr_);
    }
};

} // namespace ljf::python::ast

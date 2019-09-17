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
    // template <typename T>
    template <typename T,
              std::enable_if_t<
                  !std::is_same_v<std::decay_t<T>,
                                 Expr>> * = nullptr>
    Expr(T &&t)
    {
        expr_var_ptr_ = std::make_unique<ExprVariant>(std::forward<T>(t));
    }

    // Expr may be moved on construction.
    Expr(const Expr&) = delete;
    Expr(Expr&&) = default;

    template <typename Visitor>
    auto accept(Visitor &&visitor) const
    {
        assert(expr_var_ptr_);
        return std::visit(std::forward<Visitor>(visitor), *expr_var_ptr_);
    }
};

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
    ParenthFormExpr() {}
};

struct DictExpr : detail::EnclosureExpr
{
    using EnclosureExpr::EnclosureExpr;
};

struct UnaryExpr
{
    Token operator_;
    Expr operand_;

    UnaryExpr(Token _operator, Expr operand)
        : operator_(std::move(_operator)),
          operand_(std::move(operand)) {}
};

struct BinaryExpr
{
    Token operator_;
    Expr left_;
    Expr right_;

    BinaryExpr(Expr left, Token oper, Expr right)
        : operator_(std::move(oper)),
          left_(std::move(left)),
          right_(std::move(right)) {}
};

struct ExprVariant : std::variant<
                         StringLiteralExpr,
                         IntegerLiteralExpr,
                         IdentifierExpr,
                         ListExpr,
                         ParenthFormExpr,
                         DictExpr,
                         UnaryExpr,
                         BinaryExpr>
{
    using variant::variant;
};

} // namespace ljf::python::ast

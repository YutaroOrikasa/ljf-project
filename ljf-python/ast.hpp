#pragma once

#include "tuple"

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


} // namespace ljf::python::ast

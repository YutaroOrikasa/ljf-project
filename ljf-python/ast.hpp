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
    ParenthFormExpr(/* args */) {}
};

} // namespace ljf::python::ast

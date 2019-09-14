#pragma once
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
    template <typename... Ts>
    ListExpr(Ts...) {}
};

} // namespace ljf::python::ast

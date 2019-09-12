#pragma once
#include "token.hpp"

#include "parser.hpp"
namespace ljf::python::ast
{

class StringLiteral
{
private:
    Token token_;
public:
    StringLiteral(Token&& token) : token_(std::move(token)) {}
};

} // namespace ljf::python::ast

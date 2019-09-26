#pragma once

#include <tuple>
#include <variant>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include <cassert>

#include "../../Token.hpp"

#include "Expr.hpp"

namespace ljf::python::ast
{

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

} // namespace ljf::python::ast

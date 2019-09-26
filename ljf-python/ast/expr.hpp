#pragma once

#include <tuple>
#include <variant>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include <cassert>

#include "../Token.hpp"

#include "expr/Expr.hpp"

#include "expr/literal.hpp"
#include "expr/ident.hpp"
#include "expr/enclosure.hpp"
#include "expr/dict.hpp"
#include "expr/comprehension.hpp"

namespace ljf::python::ast
{

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

struct LambdaExpr
{
    using is_expr_impl = void;
};

struct StarExpr
{
    Expr expr_;
    using is_expr_impl = void;
    explicit StarExpr(Expr expr) : expr_(std::move(expr)) {}
};

struct Subscript
{
};

struct Slice
{
};

struct DotIdentifier : private IdentifierExpr
{
    explicit DotIdentifier(IdentifierExpr ident) : IdentifierExpr(std::move(ident)) {}
    using IdentifierExpr::name;
};

static_assert(std::is_constructible_v<DotIdentifier, IdentifierExpr>);
static_assert(std::is_copy_constructible_v<DotIdentifier>);
static_assert(std::is_copy_assignable_v<DotIdentifier>);

struct Argument
{
    struct Arg
    {
        Expr expr;
    };
    struct KeywordArg
    {
        Expr keyword, expr;
    };

    std::variant<Arg, KeywordArg, Comprehension> arg_var;

    explicit Argument(Comprehension comp) : arg_var(std::move(comp)) {}

    explicit Argument(Expr expr) : arg_var(Arg{std::move(expr)}) {}

    Argument(Expr keyword, Token eq, Expr expr)
        : arg_var(
              KeywordArg{
                  std::move(keyword),
                  std::move(expr)})
    {
        assert(eq == "=");
    }
};

using ArgList = std::vector<Argument>;
using SubscriptList = std::vector<Expr>;

using Trailer = std::variant<ArgList, SubscriptList, DotIdentifier>;

struct AtomExpr
{
    bool has_await_ = false;
    Expr atom_;
    std::vector<Trailer> trailers_;

    using is_expr_impl = void;
    AtomExpr(std::optional<Token> await_token, Expr atom, std::vector<Trailer> trailers)
        : atom_(atom), trailers_(trailers)
    {
        if (await_token)
        {
            has_await_ = true;
        }
    }
};

struct YieldExpr
{
    bool yield_from = false;
    std::optional<Expr> expr_;

    using is_expr_impl = void;
    YieldExpr() = default;
    explicit YieldExpr(Expr expr) : expr_(std::move(expr))
    {
    }

    YieldExpr(const Token &from_token, Expr expr)
        : yield_from(true),
          expr_(std::move(expr))
    {
        assert(from_token == "from");
    }
};

struct BuiltinObjectExpr : detail::SingleTokenExpr
{
    using is_expr_impl = void;
    using SingleTokenExpr::SingleTokenExpr;
};

struct SliceExpr
{
    using is_expr_impl = void;

    template <typename... Args>
    explicit SliceExpr(Args &&...) {}
};

struct ExprVariant : std::variant<
                         StringLiteralExpr,
                         IntegerLiteralExpr,
                         IdentifierExpr,
                         TupleExpr,
                         ListExpr,
                         DictExpr,
                         SetExpr,
                         UnaryExpr,
                         BinaryExpr,
                         ConditionalExpr,
                         LambdaExpr,
                         StarExpr,
                         AtomExpr,
                         YieldExpr,
                         GeneratorExpr,
                         ListComprehensionExpr,
                         DictComprehensionExpr,
                         BuiltinObjectExpr,
                         SliceExpr,
                         DictKeyValueExpr>
{
    using variant::variant;
};

using ExprList = std::vector<Expr>;

} // namespace ljf::python::ast

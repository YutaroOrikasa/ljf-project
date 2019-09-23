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
public:
    std::vector<Expr> expr_list_;

public:
    explicit EnclosureExpr(std::vector<Expr> expr_list) : expr_list_(std::move(expr_list)) {}
    EnclosureExpr() = default;
};

} // namespace detail

struct IdentifierExpr : detail::SingleTokenExpr
{
    using SingleTokenExpr::SingleTokenExpr;

    const std::string &name() const noexcept
    {
        return token().str();
    }
};

class ListExpr : detail::EnclosureExpr
{
private:
public:
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

class TupleExpr : detail::EnclosureExpr
{
private:
public:
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

/// kind of
///  1. (expr)
///  2. ()
struct ParenthFormExpr
{
    std::optional<Expr> expr_;
    using is_expr_impl = void;
    ParenthFormExpr() {}
    ParenthFormExpr(Expr expr) : expr_(std::move(expr)) {}
};

struct DictExpr : detail::EnclosureExpr
{
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

struct SetExpr : detail::EnclosureExpr
{
    using is_expr_impl = void;
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

struct Argument
{
};

struct Subscript
{
};

struct Slice
{
};

// template <typename T>
// struct ListKind : public std::vector<T>
// {
//     using ListKind::vector::vector;
// };

struct DotIdentifier : private IdentifierExpr
{
    explicit DotIdentifier(IdentifierExpr ident) : IdentifierExpr(std::move(ident)) {}
    using IdentifierExpr::name;
};

static_assert(std::is_constructible_v<DotIdentifier, IdentifierExpr>);
static_assert(std::is_copy_constructible_v<DotIdentifier>);
static_assert(std::is_copy_assignable_v<DotIdentifier>);

using IdentifierList = std::vector<IdentifierExpr>;


// represents parts of comprehension:
// if Expr
// of
// [ x * y * z for x, y, z in Expr if Expr]
struct CompIf
{
    Expr expr_;
};

// represents parts of comprehension:
// for x, y, z in Expr
// of
// [ x * y * z for x, y, z in Expr if Expr]
struct CompFor
{
    IdentifierList ident_list_;
    Expr in_expr_;
    // use shared pointer to make this class copyable
    std::shared_ptr<const std::variant<CompFor, CompIf>> comp_iter_;
};

struct Comprehension
{
    Expr expr;
    CompFor comp_for;
};

using ExprList = std::vector<Expr>;

using ArgList = std::vector<Argument>;
using SubscriptList = std::vector<Subscript>;

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
    using is_expr_impl = void;    
};

struct ComprehensionKindExpr
{
    Comprehension comp_;
    explicit ComprehensionKindExpr(Comprehension comp) : comp_(std::move(comp)) {}
};

struct GeneratorExpr : public ComprehensionKindExpr
{
    using is_expr_impl = void;    
    using ComprehensionKindExpr::ComprehensionKindExpr;
};

struct ListComprehensionExpr : public ComprehensionKindExpr
{
    using is_expr_impl = void;    
    using ComprehensionKindExpr::ComprehensionKindExpr;
};

struct DictComprehensionExpr : public ComprehensionKindExpr
{
    // using is_expr_impl = void;    
    using ComprehensionKindExpr::ComprehensionKindExpr;
};



struct ExprVariant : std::variant<
                         StringLiteralExpr,
                         IntegerLiteralExpr,
                         IdentifierExpr,
                         TupleExpr,
                         ListExpr,
                         DictExpr,
                         UnaryExpr,
                         BinaryExpr,
                         ConditionalExpr,
                         LambdaExpr,
                         StarExpr,
                         AtomExpr,
                         YieldExpr,
                         GeneratorExpr,
                         ListComprehensionExpr>
{
    using variant::variant;
};

} // namespace ljf::python::ast

#pragma once

#include <tuple>
#include <variant>
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include <cassert>

#include "expr.hpp"
#include "stmt/Stmt.hpp"

namespace ljf::python::ast
{

using StmtList = std::vector<Stmt>;

struct MultiStmt
{
    StmtList stmt_list_;
    using is_stmt_impl = void;
    explicit MultiStmt(Stmt s) : stmt_list_{std::move(s)} {}
    explicit MultiStmt(StmtList s) : stmt_list_(std::move(s)) {}
    MultiStmt() = default;
    MultiStmt(const MultiStmt &) = default;
    MultiStmt(MultiStmt &&) = default;
    MultiStmt &operator=(const MultiStmt &) = default;
    MultiStmt &operator=(MultiStmt &&) = default;
};

struct Elif
{
    Expr cond_;
    MultiStmt then_;
};

struct IfStmt
{
    Expr cond_;
    MultiStmt then_;
    std::vector<Elif> elif_;
    std::optional<MultiStmt> else_;

    using is_stmt_impl = void;
};

struct ForStmt
{
    Expr target_;
    Expr expr_to_iterate_;
    MultiStmt body_;
    using is_stmt_impl = void;
};

struct Parameter
{
    IdentifierExpr name;
    std::optional<Expr> default_value;
};

struct DefStmt
{
    IdentifierExpr funcname_;
    std::vector<Parameter> params_;
    StmtList stmt_list_;

    using is_stmt_impl = void;
};

struct ClassStmt
{
    IdentifierExpr classname_;
    std::vector<Parameter> inheritance_list_;
    StmtList stmt_list_;
    using is_stmt_impl = void;
};

struct ImportStmt
{
    using is_stmt_impl = void;
};

struct ExprStmt
{
    Expr expr_;

    using is_stmt_impl = void;
    explicit ExprStmt(Expr expr) : expr_(std::move(expr)) {}
    ExprStmt(const ExprStmt &) = default;
    ExprStmt(ExprStmt &&) = default;
    ExprStmt &operator=(const ExprStmt &) = default;
    ExprStmt &operator=(ExprStmt &&) = default;
};

struct AssignStmt
{
    std::vector<Expr> lhs_list_;
    Expr rhs_;
    using is_stmt_impl = void;
};

struct StmtVariant : std::variant<IfStmt,
                                  ForStmt,
                                  DefStmt,
                                  ClassStmt,
                                  ImportStmt,
                                  ExprStmt,
                                  AssignStmt,
                                  MultiStmt>
{
    using variant::variant;
};

} // namespace ljf::python::ast

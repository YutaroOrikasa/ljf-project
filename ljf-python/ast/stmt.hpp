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

struct IfStmt
{
    Expr cond_;
    StmtList if_;
    std::optional<StmtList> else_;

    using is_stmt_impl = void;
};

struct ForStmt
{
    Expr target_;
    Expr expr_to_iterate_;
    StmtList stmt_list_;
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
    ExprStmt(const ExprStmt&) = default;
    ExprStmt(ExprStmt&&) = default;
    ExprStmt& operator=(const ExprStmt&) = default;
    ExprStmt& operator=(ExprStmt&&) = default;
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
                                  AssignStmt>
{
    using variant::variant;
};

} // namespace ljf::python::ast
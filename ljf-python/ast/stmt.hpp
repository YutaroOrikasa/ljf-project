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

// Parameter that can take default value.
struct DefParameter
{
    IdentifierExpr name;
    std::optional<Expr> opt_default_value;
};

// Parameter such like (*a)
struct StarredParameter
{
    IdentifierExpr name;
};

// Parameter such like (**a)
struct DoubleStarredParameter
{
    IdentifierExpr name;
};

// struct Parameter : public std::variant<DefParameter, StarredParameter, DoubleStarredParameter>
// {
//     using variant::variant;
// };

using StarredParams = std::tuple<std::optional<StarredParameter>, std::optional<DoubleStarredParameter>>;

/// Parameters that appear function def statement.
struct FuncParams
{
    std::vector<DefParameter> def_params_;
    std::optional<StarredParameter> opt_starred_param_;
    std::optional<DoubleStarredParameter> opt_double_starred_param_;

    FuncParams() = default;

    explicit FuncParams(DefParameter defparam, std::vector<DefParameter> defparam_vec, std::optional<StarredParams> opt_stars)
    {
        def_params_.push_back(defparam);

        for (auto &&param : defparam_vec)
        {
            def_params_.push_back(std::move(param));
        }

        if (opt_stars)
        {
            auto [opt_starred, opt_double_starred] = std::move(*opt_stars);
            opt_starred_param_ = std::move(opt_starred);
            opt_double_starred_param_ = std::move(opt_double_starred);
        }
    }

    explicit FuncParams(std::vector<DefParameter> defparam_vec, std::optional<StarredParams> opt_stars)
        : def_params_(std::move(defparam_vec))
    {
        if (opt_stars)
        {
            auto [opt_starred, opt_double_starred] = std::move(*opt_stars);
            opt_starred_param_ = std::move(opt_starred);
            opt_double_starred_param_ = std::move(opt_double_starred);
        }
    }

    explicit FuncParams(std::vector<DefParameter> defparam_vec, bool, std::optional<StarredParams> opt_stars)
        : def_params_(std::move(defparam_vec))
    {
        if (opt_stars)
        {
            auto [opt_starred, opt_double_starred] = std::move(*opt_stars);
            opt_starred_param_ = std::move(opt_starred);
            opt_double_starred_param_ = std::move(opt_double_starred);
        }
    }

    explicit FuncParams(std::vector<DefParameter> defparam_vec)
        : def_params_(std::move(defparam_vec)) {}

    explicit FuncParams(std::optional<StarredParameter> sp, std::optional<DoubleStarredParameter> dsp)
        : opt_starred_param_(std::move(sp)),
          opt_double_starred_param_(std::move(dsp))
    {
    }

    explicit FuncParams(std::optional<DoubleStarredParameter> dsp)
        : opt_double_starred_param_(std::move(dsp))
    {
    }
};

struct DefStmt
{
    IdentifierExpr funcname_;
    FuncParams params_;
    MultiStmt body_;

    using is_stmt_impl = void;
};

struct ClassStmt
{
    IdentifierExpr classname_;
    std::vector<DefParameter> inheritance_list_;
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

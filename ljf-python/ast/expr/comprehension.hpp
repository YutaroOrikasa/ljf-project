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

struct CompIf;
struct CompFor;
using CompIter = std::variant<CompFor, CompIf>;

// represents parts of comprehension:
// if Expr
// of
// [ x * y * z for x, y, z in Expr if Expr]
struct CompIf
{
    Expr test_;
    // use pointer because CompIter is incomplete type.
    // use shared pointer to make this class copyable.
    std::shared_ptr<const CompIter> comp_iter_;

    // define ctor after CompFor is defined.
    // std::optional<CompIter> is incomplete type here
    // because CompIter is include incomplete type CompFor.
    CompIf(Expr test, std::optional<CompIter> comp_iter);
};

// represents parts of comprehension:
// for x, y, z in Expr
// of
// [ x * y * z for x, y, z in Expr if Expr]
struct CompFor
{
    Expr target_list_;
    Expr in_expr_;
    // use pointer because CompIter is incomplete type.
    // use shared pointer to make this class copyable.
    std::shared_ptr<const CompIter> comp_iter_;

    CompFor(Expr target_list, Expr in_expr, std::optional<CompIter> &&comp_iter)
        : target_list_(target_list),
          in_expr_(in_expr)
    {
        if (comp_iter)
        {
            comp_iter_ = std::make_shared<CompIter>(*comp_iter);
        }
    }
};

inline CompIf::CompIf(Expr test, std::optional<CompIter> comp_iter)
    : test_(std::move(test))
{
    if (comp_iter.has_value())
    {
        comp_iter_ = std::make_shared<CompIter>(*comp_iter);
    }
}

struct Comprehension
{
    Expr expr;
    CompFor comp_for;
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
    using is_expr_impl = void;
    using ComprehensionKindExpr::ComprehensionKindExpr;
};

} // namespace ljf::python::ast

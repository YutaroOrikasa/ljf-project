#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>
#include <vector>

#include <cassert>

#include "../../Token.hpp"

#include "Expr.hpp"

/// expr ast class for tuple, list, and set.
/// dict ast is defined in dict.hpp

namespace ljf::python::ast {

namespace detail {
    class EnclosureExpr {
    public:
        std::vector<Expr> expr_list_;

    public:
        explicit EnclosureExpr(std::vector<Expr> expr_list)
            : expr_list_(std::move(expr_list)) {}
        EnclosureExpr() = default;
    };
} // namespace detail

class TupleExpr : public detail::EnclosureExpr {
private:
public:
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

class ListExpr : public detail::EnclosureExpr {
private:
public:
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

struct SetExpr : detail::EnclosureExpr {
    using is_expr_impl = void;
    using EnclosureExpr::EnclosureExpr;
};

} // namespace ljf::python::ast

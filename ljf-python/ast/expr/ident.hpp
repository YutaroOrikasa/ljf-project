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

namespace ljf::python::ast {

struct IdentifierExpr : public detail::SingleTokenExpr {
    using is_expr_impl = void;
    using SingleTokenExpr::SingleTokenExpr;

    const std::string &name() const noexcept { return token().str(); }
};

} // namespace ljf::python::ast

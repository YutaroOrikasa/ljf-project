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

struct DictKeyValueExpr;

class DictExpr
{
public:
    std::vector<DictKeyValueExpr> expr_list_;

public:
    using is_expr_impl = void;
    explicit DictExpr(std::vector<DictKeyValueExpr> expr_list) : expr_list_(std::move(expr_list)) {}
    DictExpr() = default;
};

} // namespace ljf::python::ast

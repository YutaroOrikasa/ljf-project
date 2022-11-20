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

struct KeyValue {
    Expr key;
    Expr value;
};

struct DictKeyValueExpr {
    using is_expr_impl = void;
    using DoubleStaredExpr = Expr;
    std::variant<KeyValue, DoubleStaredExpr> key_datum_;

    DictKeyValueExpr(Expr key, Expr value)
        : key_datum_(KeyValue{std::move(key), std::move(value)}) {}

    DictKeyValueExpr(const Token &double_star, Expr expr)
        : key_datum_(std::in_place_index<1>, std::move(expr)) {
        assert(double_star == "**");
    }
};

class DictExpr {
public:
    std::vector<DictKeyValueExpr> expr_list_;

public:
    using is_expr_impl = void;
    explicit DictExpr(std::vector<DictKeyValueExpr> expr_list)
        : expr_list_(std::move(expr_list)) {}
    DictExpr() = default;
};

} // namespace ljf::python::ast



#pragma once

// copyid from https://github.com/python/cpython/blob/3.5/Grammar/Grammar

// This file is C++ port of python grammer file.

// # Grammar for Python

// # Note:  Changing the grammar specified in this file will most likely
// #        require corresponding changes in the parser module
// #        (../Modules/parsermodule.c).  If you can't make the changes to
// #        that module yourself, please co-ordinate the required changes
// #        with someone who can; ask around on python-dev for help.  Fred
// #        Drake <fdrake@acm.org> will probably be listening there.

// # NOTE WELL: You should also follow all the steps listed at
// # https://docs.python.org/devguide/grammar.html

#include <type_traits>
#include <variant>
#include <vector>

#include "parser.hpp"

namespace ljf::python::parser
{

struct EmptySExpr
{
};

class SExpr
{
public:
    using SExprList = std::vector<SExpr>;

private:
    std::variant<EmptySExpr, Token, SExprList, ast::Expr> variant_;

public:
    template <typename... Args, std::enable_if_t<sizeof...(Args) >= 2> * = nullptr>
    SExpr(Args &&... args)
        : variant_(std::in_place_type<SExprList>,
                   {SExpr(std::forward<Args>(args))...}) {}

    SExpr() = default;

    SExpr(const SExpr &) = default;
    SExpr(SExpr &&other) = default;
    SExpr &operator=(const SExpr &) = default;
    SExpr &operator=(SExpr &&other) = default;

    explicit SExpr(Token token) : variant_(std::in_place_type<Token>, std::move(token)) {}

    explicit SExpr(ast::Expr expr) : variant_(std::in_place_type<ast::Expr>, std::move(expr)) {}

    template <typename T>
    explicit SExpr(std::vector<T> vec) : variant_(SExprList(vec.begin(), vec.end())) {}

    template <typename T>
    explicit SExpr(std::optional<T> opt)
    {
        if (opt)
        {
            *this = make_from_variant<SExpr>(std::move(opt.value()));
        }
    }

    template <typename... Ts>
    explicit SExpr(std::variant<Ts...> var) : SExpr(make_from_variant<SExpr>(std::move(var))) {}

    template <typename... Ts>
    explicit SExpr(std::tuple<Ts...> tup) : SExpr(make_from_variant<SExpr>(std::move(tup))) {}

    template <typename V>
    auto accept(V &&visitor) const
    {
        return std::visit(std::forward<V>(visitor), variant_);
    }
};

using SExprList = SExpr::SExprList;

inline auto printer(const std::string &str)
{
    return Parser([str](const auto &token_steram) {
        (void)token_steram;
        std::cerr << str << "\n";
        return Separator();
    });
}

inline auto prompter(const std::string &str)
{
    return Parser([str](auto &token_steram) {
        token_steram.prompt(str);
        return Separator();
    });
}

namespace impl
{
struct Opt
{
    template <typename Parser>
    constexpr auto operator[](Parser &&p) const noexcept
    {
        return option(std::forward<Parser>(p));
    }

    template <size_t N>
    constexpr auto operator[](const char (&str)[N]) const noexcept
    {
        return option_str(str);
    }
};

template <size_t>
struct Many
{
};

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<0>)
{
    return many(std::forward<Parser>(p));
}

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<1>)
{
    return p + many(p);
}

inline constexpr Opt opt;

inline constexpr Many<0> _many;
inline constexpr Many<1> _many1;

} // namespace impl

ParserPlaceHolder<SExpr> make_python_grammer_parser();

} // namespace ljf::python::parser

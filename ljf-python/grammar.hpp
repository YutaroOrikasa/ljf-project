

#pragma once

// copied from https://github.com/python/cpython/blob/3.5/Grammar/Grammar

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
#include <utility>
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
} // namespace ljf::python::parser

namespace ljf::python::grammar
{

const parser::Parser NEWLINE = parser::newline;
const parser::Parser ENDMARKER = parser::eof;
const parser::Parser NAME = parser::identifier;
const parser::Parser INDENT = parser::indent;
const parser::Parser DEDENT = parser::dedent;
const parser::Parser NUMBER = parser::integer_literal;
const parser::Parser STRING = parser::string_literal;

namespace detail
{
/// equivalent to optional(p + many(sep + p) + optional(sep))
/// result type: std::pair{vector<result of p>, bool:ends_with_sep}
template <typename Parser, typename SepParser>
constexpr auto sep_many_optsep(Parser p, SepParser sep)
{
    using parser::option;
    auto parser = p + option(sep);

    return parser::Parser(
        [parser](auto &&token_stream) {
            using namespace parser;

            using vec_value_ty = result_content_t<Parser, decltype(token_stream)>;
            using vec_ty = std::vector<vec_value_ty>;
            using pair_ty = std::pair<vec_ty, bool>;
            using ResultTy = Result<pair_ty>;

            vec_ty vec;

            bool ends_with_sep = false;
            while (true)
            {
                auto init_pos = token_stream.current_position();

                auto result = parser(token_stream);
                if (LL1_parser_fatally_failed(result, init_pos, token_stream))
                {
                    return ResultTy(result.extract_error_ptr());
                }

                // parser consumed no tokens, only lookahead was done.
                // (p sep) (p sep) ... (p sep); ends_with_sep=true were parsed
                // or
                // nothing; ends_with_sep=false  were parsed.
                // finish sep_many_optsep()
                if (result.failed())
                {
                    return ResultTy({std::move(vec), ends_with_sep});
                }

                auto [parsed, opt_sep] = result.extract_success();
                vec.push_back(std::move(parsed));

                if (!opt_sep)
                {
                    // (p sep) (p sep) ... p; were parsed.
                    return ResultTy({std::move(vec), false});
                }

                ends_with_sep = true;
            }
        });
}
} // namespace detail

parser::ParserPlaceHolder<parser::SExpr> make_python_grammer_parser();
parser::ParserPlaceHolder<ast::Expr> make_python_eval_input_parser();

} // namespace ljf::python::grammar



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
#include <optional>
#include <tuple>

#include "parser.hpp"

namespace ljf::python::parser
{

inline auto printer(const std::string &str)
{
    return Parser([str](const auto &token_steram)
                  {
                      (void)token_steram;
                      std::cerr << str << "\n";
                      return Separator();
                  });
}

inline auto prompter(const std::string &str)
{
    return Parser([str](auto &token_steram)
                  {
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
        [parser](auto &&token_stream)
        {
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

/// parse `p sep ... sep p [sep [end]]`
/// equivalent to many(p + sep) + [p | end]
/// result type: std::tuple{vector<result of p>, bool:ends_with_sep, std::optional<result of end>}
template <typename Parser, typename SepParser, typename EndParser>
constexpr auto sep_many_optsep_optend(Parser p, SepParser sep, EndParser end)
{

    return parser::Parser(
        [=](auto &&token_stream)
        {
            using namespace parser;

            using vec_value_ty = result_content_t<Parser, decltype(token_stream)>;
            using end_result_ty = result_content_t<EndParser, decltype(token_stream)>;
            using vec_ty = std::vector<vec_value_ty>;
            using tuple_ty = std::tuple<vec_ty, bool, std::optional<end_result_ty>>;
            using ResultTy = Result<tuple_ty>;

            vec_ty vec;

            bool ends_with_sep = false;
            while (true)
            {
                // parser DFA:
                // =>(0)-> p(1)/end(f)/(f) =>(1)-> sep(0)/(f)

                // try to parse p|end
                auto result = option(p | end)(token_stream);
                if (result.failed())
                {
                    return ResultTy(result.extract_error_ptr());
                }

                if (!result.success().has_value())
                {
                    break;
                }

                auto var = result.extract_success().value();
                if (var.index() == 1)
                {
                    // `end' matched
                    return ResultTy({std::move(vec), ends_with_sep, std::get<1>(var)});
                }

                // `p' matched
                vec.push_back(std::get<0>(var));

                ends_with_sep = false;

                // try to parse sep

                auto sep_result = option(sep)(token_stream);
                if (!sep_result)
                {
                    return ResultTy(sep_result.extract_error_ptr());
                }

                if (!sep_result.success().has_value())
                {
                    break;
                }

                ends_with_sep = true;
            }
            return ResultTy({std::move(vec), ends_with_sep, std::nullopt});
        });
}
} // namespace detail

parser::ParserPlaceHolder<ast::Expr> make_python_eval_input_parser();

} // namespace ljf::python::grammar

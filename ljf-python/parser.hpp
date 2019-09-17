#pragma once

#include <any>

#include "tokenizer.hpp"
#include "parser-combinator.hpp"

#include "ast.hpp"

// TODO
// - add binary +-*/ parser
// - add simple S Expr printer

namespace ljf::python::parser
{

template <typename TResult = std::any>
using ParserPlaceHolder = PlaceHolder<TResult, TokenStream<std::istream>>;
// inline constexpr Parser XXX = [](auto &&token_stream) {

// };
// inline constexpr auto XXX = [](auto YYY) {
//     return Parser(
//         [=](auto &&token_stream) {

//         });
// };

// return type: kind of Result<std::optional<T>>
template <typename T>
auto make_optional_result(const T &t)
{
    using result_content_type = std::optional<std::decay_t<T>>;
    return Result<result_content_type>(result_content_type(t));
}
// return type: kind of Result<std::optional<T>>
template <typename T>
auto make_optional_result()
{
    using result_content_type = std::optional<std::decay_t<T>>;
    return Result<result_content_type>(result_content_type());
}

// return type: kind of Result<std::optional<T>>
template <typename T>
auto make_optional_error_result(std::unique_ptr<Error> e)
{
    using result_content_type = std::optional<std::decay_t<T>>;
    return Result<result_content_type>(std::move(e));
}

// result: Result<T>
// return type: kind of Result<std::optional<T>>
template <typename T>
auto to_optional_result(Result<T> &&result)
{
    if (result.failed())
    {
        Result<std::optional<T>>(result.extract_error_ptr());
    }

    return Result<std::optional<T>>(std::optional<T>(result.extract_success()));
}

// result: Result<T>
// return type: kind of Result<std::optional<T>>
// returned result is not failed but the content std::optional<T> is empty.
template <typename T>
auto to_empty_optional_result(Result<T> &&result)
{
    return Result<std::optional<T>>(std::optional<T>());
}

inline constexpr auto option_str = [](auto str) {
    return Parser(
        [=](auto &&token_stream) {
            if (token_stream.peek().str() == str)
            {
                return make_optional_result(token_stream.read());
            }
            return make_optional_result<decltype(token_stream.read())>();
        });
};

inline constexpr auto option = [](auto parser) {
    return Parser(
        [=](auto &&token_stream) {
            auto initial_pos = token_stream.current_position();
            auto result = parser(token_stream);
            // if result.failed() && token_stream.current_position() == initial_pos
            // it is not error because this is the part of LL1 parser.
            if (result.failed() && token_stream.current_position() == initial_pos)
            {
                return to_empty_optional_result(std::move(result));
            }
            return to_optional_result(std::move(result));
        });
};

inline constexpr auto read_if = [](auto &&pred, auto &&... error_args) {
    return Parser(
        [=](auto &&token_stream) {
            using result_content_type = std::decay_t<decltype(token_stream.read())>;
            if (pred(token_stream.peek()))
            {
                return Result<result_content_type>(token_stream.read());
            }

            return make_error_result<result_content_type>(token_stream.peek(),
                                                          std::forward<decltype(error_args)>(error_args)...);
        });
};

inline constexpr auto string = [](auto &&str) {
    return read_if([=](const Token &token) {
        return token.str() == str;
    });
};

inline constexpr auto separator = [](auto &&str) {
    return result_type<Separator> <<= string(str);
};

inline constexpr auto operator""_p(const char *str, size_t)
{
    return string(str);
}

// create a parser that returns Result<Separator>.
// The result Separator will be discarded when used with sequence.
// For example, parser = identifier + ","_sep + identifier;
// type of parser(stream).success() is std::tuple<Identifier, Identifier>,
// NOT std::tuple<Identifier, Separator, Identifier>.
//
// return type: Parser<?>
inline constexpr auto operator""_sep(const char *str, size_t)
{
    return separator(str);
}

// return type: Sequence<Parser<?>...>
template <typename Parser, size_t N>
constexpr auto operator+(Parser &&parser, const char (&str)[N])
{
    return std::forward<Parser>(parser) + string(str);
}

// return type: Choice<Parser<?>...>
template <typename Parser, size_t N>
constexpr auto operator|(Parser &&parser, const char (&str)[N])
{
    return std::forward<Parser>(parser) | string(str);
}

constexpr auto token(token_category cat)
{
    return read_if([=](const Token &token) {
        return token.category() == cat;
    });
};

inline constexpr Parser eof = token(token_category::EOF_TOKEN);

inline constexpr Parser identifier = result_type<ast::IdentifierExpr> <<= token(token_category::IDENTIFIER);

inline constexpr Parser string_literal =
    result_type<ast::StringLiteralExpr> <<= read_if(
        [](const Token &token) {
            return token.is_string_literal();
        });
inline constexpr Parser integer_literal =
    result_type<ast::IntegerLiteralExpr> <<= read_if(
        [](const Token &token) {
            return token.is_integer_literal();
        });

inline constexpr Parser literal = string_literal | integer_literal;

auto make_expr_parser()
{
    using std::any;

    constexpr auto as_expr = converter(make_from_variant<ast::Expr>);

    using ast::Expr;
    ParserPlaceHolder<Expr> expression;

    // Enclosures
    ParserPlaceHolder<ast::ParenthFormExpr> parenth_form;
    // ParserPlaceHolder<ast::ListExpr> list_display;
    // ParserPlaceHolder<ast::DictExpr> dict_display;
    // ParserPlaceHolder<ast::>

    // Atoms
    Parser enclosure = parenth_form /* | list_display | dict_display  | set_display
               | generator_expression | yield_atom */
        ;
    const Parser atom = identifier | literal /*  | enclosure */;

    /* parenth_form = "("_sep + option(starred_expression) + ")"_sep; */
    parenth_form = "("_sep + /* option(expression) + */ ")"_sep;
    // list_display = "["_sep + "]"_sep;
    // dict_display = "{"_sep + "}"_sep;

    /* attributeref ::=  primary "." identifier */
    /* subscription ::=  primary "[" expression_list "]" */
    /* slice */
    /* call */
    const Parser primary = atom /* | attributeref | subscription | slicing | call */;

    ParserPlaceHolder<ast::Expr> u_expr;
    /* const Parser power =  (await_expr | primary) ["**" u_expr] */
    const Parser power = primary /* + option("**"_p + u_expr) */;

    u_expr = power | ( result_type<ast::UnaryExpr> <<= "-"_p + u_expr | "+"_p + u_expr | "~"_p + u_expr);

    expression = as_expr <<= u_expr;

    // // Boolean operations
    // constexpr Parser or_test = and_test | or_test "or" and_test;
    // // constexpr Parser and_test =  not_test | and_test "and" not_test;
    // // constexpr Parser not_test =  comparison | "not" not_test;

    // // Conditional expressions
    // constexpr Parser conditional_expression = or_test + option("if"_sep + or_test + "else"_sep + expression);
    // constexpr Parser expression = conditional_expression /* | lambda_expr */;
    // // expression_nocond      ::=  or_test | lambda_expr_nocond

    // // Expression lists
    // constexpr Parser starred_item = expression /* | "*" or_expr */;
    // // constexpr Parser expression_list  = expression + ( ","_sep + expression) * option_str(",");
    // // constexpr Parser starred_list  = starred_item("," starred_item) * [","];
    // constexpr Parser starred_expression = expression | many(starred_item + ","_sep) + option(starred_item);

    // constexpr Parser statement = many(atom);

    // assert(expression.has_parser());
    return expression;
}

} // namespace ljf::python::parser

#pragma once

#include <any>

#include "tokenizer.hpp"
#include "parser-combinator.hpp"

#include "ast/expr.hpp"

// TODO
// - add binary +-*/ parser
// - add simple S Expr printer

namespace ljf::python::parser
{

template <typename TResult = std::any>
using ParserPlaceHolder = PlaceHolder<TResult, IStreamTokenStream>;
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
        return Result<std::optional<T>>(result.extract_error_ptr());
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

inline constexpr auto option = [](auto parser)
{
    return Parser(
        [=](auto &&token_stream)
        {
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

inline constexpr auto read_if = [](auto &&pred, auto &&...error_args)
{
    return Parser(
        [=](auto &&token_stream)
        {
            using result_content_type = std::decay_t<decltype(token_stream.read())>;
            if (pred(token_stream.peek()))
            {
                return Result<result_content_type>(token_stream.read());
            }

            return make_error_result<result_content_type>(token_stream.peek(),
                                                          error_args...);
        });
};

// /// usage:
// /// parser = result_type<std::variant<T0, T1>>
// ///     <<= ctor_arg(std::in_place_type<T0>) + parser0 |
// ///       | ctor_arg(std::in_place_type<T1>) + parser1;
// inline constexpr auto ctor_arg = [](auto arg) {
//     return Parser(
//         [=](auto&& token_stream) {
//             return arg;
//         });
// };

inline constexpr auto string = [](auto &&str)
{
    return read_if([=](const Token &token)
                   { return token.str() == str; },
                   "expected ", str, " but not given");
};

inline constexpr auto option_str = [](auto &&str)
{
    return option(string(std::forward<decltype(str)>(str)));
};

inline constexpr auto separator = [](auto &&parser)
{
    return result_type<Separator> <<= parser;
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
    return separator(string(str));
}

// return type: Sequence<Parser<?>...>
template <typename Parser, typename charT>
constexpr auto operator+(Parser &&parser, const charT *str)
{
    return std::forward<Parser>(parser) + string(str);
}

// return type: Choice<Parser<?>...>
template <typename Parser, typename charT>
constexpr auto operator|(Parser &&parser, const charT *str)
{
    return std::forward<Parser>(parser) | string(str);
}

namespace detail
{
/// (T first, std::vector<U>) -> std::vector<T>
inline constexpr auto fold_left_to_vec = [](auto &&first, auto &&vec)
{
    using T = std::decay_t<decltype(first)>;
    std::vector<T> vec_ret;
    vec_ret.reserve(1 + vec.size());

    vec_ret.push_back(first);
    // This forward is exists for move elements.
    // If vec is vector<T>& , copy and move.
    // If vec is vector<T>&&, move and move.
    auto vec2 = std::forward<decltype(vec)>(vec);
    for (auto &&elem : vec2)
    {
        vec_ret.push_back(std::move(elem));
    }
    return vec_ret;
};
} // namespace detail

namespace abbrev
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

template <size_t, bool unify = false>
struct Many
{
};

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<0>)
{
    return many(std::forward<Parser>(p));
}

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<1, false>)
{
    return p + many(p);
}

// unify = true
template <typename Parser>
constexpr auto operator*(Parser &&p, Many<1, true>)
{
    return converter(detail::fold_left_to_vec) <<= p + many(p);
}

inline constexpr Opt opt;

inline constexpr Many<0> _many;
inline constexpr Many<1> _many1;
inline constexpr Many<1, true> _many1_unify;

static constexpr auto sep = [](auto &&parser)
{
    return separator(std::forward<decltype(parser)>(parser));
};
} // namespace abbrev

constexpr auto token(token_category cat)
{
    return read_if([=](const Token &token)
                   { return token.category() == cat; },
                   "expected ", cat, " but not given");
};

inline constexpr Parser eof = token(token_category::EOF_TOKEN);

inline constexpr Parser newline = token(token_category::NEWLINE);

inline constexpr Parser indent = token(token_category::INDENT);
inline constexpr Parser dedent = token(token_category::DEDENT);

inline constexpr Parser identifier = result_type<ast::IdentifierExpr> <<= token(token_category::IDENTIFIER);

inline constexpr Parser string_literal =
    result_type<ast::StringLiteralExpr> <<= read_if(
        [](const Token &token)
        {
            return token.is_string_literal();
        });
inline constexpr Parser integer_literal =
    result_type<ast::IntegerLiteralExpr> <<= read_if(
        [](const Token &token)
        {
            return token.is_integer_literal();
        });

inline constexpr Parser literal = string_literal | integer_literal;

} // namespace ljf::python::parser

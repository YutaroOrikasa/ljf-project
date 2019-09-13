#include "tokenizer.hpp"
#include "parser-combinator.hpp"

namespace ljf::python::parser
{
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
        Result<std::optional<T>>(std::optional<T>(result.extract_error_ptr()));
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

inline constexpr auto optional_str = [](auto str) {
    return Parser(
        [=](auto &&token_stream) {
            if (token_stream.peek().str() == str)
            {
                return make_optional_result(token_stream.read());
            }
            return make_optional_result<decltype(token_stream.read())>();
        });
};

inline constexpr auto optional = [](auto parser) {
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

            return make_error_result<result_content_type>(std::forward<decltype(error_args)>(error_args)...);
        });
};

inline constexpr auto string = [](auto &&str) {
    return read_if([=](const Token &token) {
        return token.str() == str;
    });
};

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

// This definition is incomplete.
inline constexpr Parser identifier = token(token_category::ANY_OTHER);

inline constexpr Parser literal = read_if([](const Token &token) {
    return token.is_string_literal() || token.is_integer_literal();
});

// inline constexpr Parser enclosure = parenth_form | list_display | dict_display | set_display
//                | generator_expression | yield_atom;

inline constexpr auto atom = identifier | literal/*  | enclosure */;

inline constexpr Parser statement = atom;// many(atom);

} // namespace ljf::python::parser


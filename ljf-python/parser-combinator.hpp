#include <utility>
#include <type_traits>
#include <vector>
#include <optional>
#include <variant>
#include <string>
#include <memory>
#include <functional>
#include <tuple>

#include <cassert>

// helpers
namespace ljf::python::parser::detail
{

template <typename T, typename Tuple, typename... Args>
constexpr T make_from_tuple_and_args(Tuple &&tuple, Args &&... args)
{
    return std::make_from_tuple<T>(
        std::tuple_cat(
            std::forward<Tuple>(tuple),
            std::tuple(std::forward<Args>(args)...)));
}

} // namespace ljf::python::parser::detail

namespace ljf::python::parser
{

template <typename... P>
class Sequence;

/// ordered choice
template <typename... P>
class Choice;

struct Separator
{
    template <typename... Args>
    explicit Separator(Args...) {}
};

namespace detail
{

// wrap T with std::tuple
// but if T is Separator, return empty tuple
template <typename T>
auto to_tuple(T &&t)
{
    if constexpr (std::is_same_v<std::decay_t<T>, Separator>)
    {
        return std::tuple();
    }
    else
    {
        return std::tuple<T>(std::forward<T>(t));
    }
}
// return type: std::tuple<...>
template <typename... Ts>
auto discard_separator(Ts &&... ts)
{
    return std::tuple_cat(
        to_tuple(
            std::forward<Ts>(ts))...);
}

} // namespace detail

class Error
{
private:
    // The token that caused parsing error.
    Token token_;
    std::string msg_;

public:
    // token: The token that caused parsing error.
    Error(const Token &token, const std::string &&msg)
        : token_(token),
          msg_(msg) {}

    // token: The token that caused parsing error.
    explicit Error(const Token &token) : Error(token, ""){};

    virtual ~Error() = default;

    // return: The token that caused parsing error.
    virtual const Token &token() const
    {
        return token_;
    }

    virtual const std::string &str() const
    {
        return msg_;
    }
};

template <typename Out>
Out &operator<<(Out &out, const Error &e)
{
    out << "Error: ";
    if (!e.str().empty())
    {
        out << e.str() << ": ";
    }

    auto &token = e.token();
    if (token.is_eof())
    {
        out << "error token = <EOF>";
    }
    else if (token.is_newline())
    {
        out << "error token = <NEWLINE>";
    }
    else if (token.is_indent())
    {
        out << "error token = <INDENT>";
    }
    else if (token.is_dedent())
    {
        out << "error token = <DEDENT>";
    }
    else if (token.is_invalid())
    {
        out << "invalid token `" << token.str() << "`: ";
        out << token.error_message();
    }
    else
    {
        out << "error token = `" << token.str() << "`";
    }

    return out;
}

template <typename T>
class Result
{
private:
    std::variant<T, std::unique_ptr<Error>> value_;

public:
    using success_type = T;
    explicit Result(T &&t) : value_(std::move(t)) {}
    explicit Result(const T &t) : value_(t) {}
    explicit Result(std::unique_ptr<Error> e) : value_(std::move(e)) {}

    bool failed() const noexcept
    {
        return !std::holds_alternative<T>(value_);
    }

    explicit operator bool() const noexcept
    {
        return !failed();
    }

    const T &success() const &noexcept
    {
        assert(!failed());
        return std::get<T>(value_);
    }

    T success() &&
    {
        assert(!failed());
        return std::move(std::get<T>(value_));
    }

    T extract_success()
    {
        assert(!failed());
        return std::move(std::get<T>(value_));
    }

    const Error &error() const noexcept
    {
        assert(failed());
        return *std::get<std::unique_ptr<Error>>(value_);
    }

    std::unique_ptr<Error> error_ptr() &&
    {
        assert(failed());
        return std::move(std::get<std::unique_ptr<Error>>(value_));
    }

    std::unique_ptr<Error> extract_error_ptr()
    {
        assert(failed());
        return std::move(std::get<std::unique_ptr<Error>>(value_));
    }

    template <size_t I>
    const auto &get() const
    {
        return std::get<I>(success());
    }
    template <typename F>
    auto visit(F &&visitor) const
    {
        return visit_impl(std::forward<F>(visitor), success());
    }

private:
    template <typename F, typename U>
    static auto visit_impl(F &&visitor, const U &t)
    {
        return std::forward<F>(visitor)(t);
    }

    template <typename F, typename... Ts>
    static auto visit_impl(F &&visitor, const std::variant<Ts...> &t)
    {
        return std::visit(std::forward<F>(visitor), t);
    }
};

template <size_t I, typename T>
const auto &get(const Result<T> &result)
{
    return std::get<I>(result.success());
}

template <typename T, typename... Args>
Result<T> make_error_result(Args &&... args)
{
    return Result<T>(std::make_unique<Error>(std::forward<Args>(args)...));
}

/// F should be deduced by constructor argument, void is dummy.
template <typename TResult = void, typename F = void>
class Parser
{
    F f_;
    using this_type = Parser<TResult, F>;

public:
    /*implicit*/ constexpr Parser(F &&f) : f_(std::move(f)) {}
    /*implicit*/ constexpr Parser(const F &f) : f_(f) {}

    template <typename TokenStream>
    auto operator()(TokenStream &&ts) const
    {
        // We don't create Result<Result<T>>, so use to_result() helper.
        // Type of result is flatten.
        Result result = to_result(f_(std::forward<TokenStream>(ts)));
        if constexpr (std::is_same_v<TResult, void>)
        {

            return Result(std::move(result));
        }
        else
        {
            if (result)
            {
                return Result(TResult(result.extract_success()));
            }
            else
            {
                return Result<TResult>(result.extract_error_ptr());
            }
        }
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const
    {
        return Sequence<this_type, Parser2>(*this, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const
    {
        return Choice<this_type, Parser2>(*this, parser2);
    }

private:
    /*** to_result() functions ***/

    template <typename T>
    static auto to_result(T &&t)
    {
        return Result<T>(std::move(t));
    }

    template <typename T>
    static auto to_result(Result<T> result)
    {
        return std::move(result);
    }

    template <typename T>
    static auto to_result(T &t) = delete;

    /*** for ResultType<T> ***/

    template <typename T>
    friend struct ResultType;

    constexpr auto extract_func()
    {
        return std::move(f_);
    }
};

template <typename... Ps>
class Sequence
{
    std::tuple<Ps...> parser_tuple_;
    static constexpr size_t tuple_size_ = sizeof...(Ps);
    using this_type = Sequence<Ps...>;

public:
    constexpr Sequence(const Ps &... parsers) : parser_tuple_(parsers...) {}

    template <typename TokenStream>
    auto operator()(TokenStream &token_stream) const
    {
        return parse<0>(token_stream);
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const
    {
        // make Sequence<Ps..., Parser2> from this and parser2
        // We don't make nested sequence type such like `Sequence<Sequence<Ps...>, Parser2>`
        return detail::make_from_tuple_and_args<Sequence<Ps..., Parser2>>(parser_tuple_, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const
    {
        return Choice<this_type, Parser2>(*this, parser2);
    }

private:
    // return type: Result<std::tuple<...>>
    template <size_t I, typename TokenStream, typename... Results>
    auto parse(TokenStream &token_stream, Results &&... results) const
    {
        if constexpr (I == tuple_size_)
        {
            return Result(
                detail::discard_separator(
                    std::move(results)...));
        }
        else
        {
            auto result = std::get<I>(parser_tuple_)(token_stream);
            if (result.failed())
            {
                using ResultTy = decltype(parse<I + 1>(token_stream, std::move(results)..., std::move(result).success()));
                // return error
                return ResultTy(std::move(result).error_ptr());
            }

            return parse<I + 1>(token_stream, std::move(results)..., std::move(result).success());
        }
    }
};

template <typename... Ps>
class Choice
{
    std::tuple<Ps...> parser_tuple_;
    static constexpr size_t tuple_size_ = sizeof...(Ps);
    using this_type = Choice<Ps...>;

public:
    constexpr Choice(const Ps &... parsers) : parser_tuple_(parsers...) {}

    template <typename TokenStream>
    auto operator()(TokenStream &token_stream) const
    {
        return parse<0>(token_stream);
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const
    {
        return Sequence<this_type, Parser2>(*this, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const
    {
        // We don't make nested type Choice<Choice<Ps...>, Parser2>
        // as same as Sequence<?>::operator+().
        return detail::make_from_tuple_and_args<Choice<Ps..., Parser2>>(parser_tuple_, parser2);
    }

private:
    // return type: Result<std::variant<...>>
    template <size_t I, typename TokenStream>
    auto parse(TokenStream &token_stream) const
    {
        using variant_type = std::variant<
            std::decay_t<
                typename decltype(std::declval<Ps>()(token_stream))::success_type>...>;
        using ResultTy = Result<variant_type>;

        static_assert(I < tuple_size_);

        auto current_pos = token_stream.current_position();
        auto result = std::get<I>(parser_tuple_)(token_stream);

        if (!result.failed())
        {
            return ResultTy(variant_type(std::in_place_index<I>, std::move(result).success()));
        }

        // This is LL1 parser.
        // If current_position was changed, it is parse error.
        if (token_stream.current_position() != current_pos)
        {
            // return error
            return ResultTy(std::move(result).error_ptr());
        }
        else
        {
            if constexpr (I + 1 < tuple_size_)
            {
                // try next parser
                return parse<I + 1>(token_stream);
            }
            else
            {
                // There are no parser left.
                // Return the last result (== error)
                return ResultTy(std::move(result).error_ptr());
            }
        }
    }
};

template <typename Parser, typename TokenStream>
using result_content_t = std::decay_t<
    decltype(
        std::declval<Parser>()(
            std::declval<TokenStream &>())
            .success())>;

/// return type: Parser<std::vector<?>,?>
/// The parser many() generates will not return failed result.
template <typename P>
constexpr auto many(P &&parser)
{
    return Parser(
        [parser](auto &&token_stream) {
            using result_ty = result_content_t<P, decltype(token_stream)>;
            std::vector<result_ty> vec;
            while (auto result = parser(token_stream))
            {
                vec.push_back(result.extract_success());
            }
            return vec;
        });
}

template <typename R>
struct ResultType
{
    template <typename R0, typename F>
    constexpr auto operator<<=(Parser<R0, F> &&p) const
    {
        return Parser<R, F>(p.extract_func());
    }

    template <typename F>
    constexpr auto operator<<=(F &&f) const
    {
        return Parser<R, F>(std::move(f));
    }
};

template <typename T>
inline constexpr auto result_type = ResultType<T>();

template <typename TResult, class TokenStream>
class PlaceHolder
{
private:
    using func_type = std::function<TResult(TokenStream &)>;
    Parser<TResult, func_type> parser_;

public:
    template <typename F>
    PlaceHolder &operator=(const Parser<void, F> &parser)
    {
        parser_ = result_type<TResult> <<= parser;
    }

    template <typename F>
    PlaceHolder &operator=(const Parser<TResult, F> &parser)
    {
        parser_ = parser;
    }

    auto operator()(TokenStream &ts) const
    {
        return parser_(ts);
    }

    template <typename Parser2>
    auto operator+(const Parser2 &parser2) const
    {
        return Sequence<PlaceHolder, Parser2>(*this, parser2);
    }

    template <typename Parser2>
    auto operator|(const Parser2 &parser2) const
    {
        return Choice<PlaceHolder, Parser2>(*this, parser2);
    }
};

} // namespace ljf::python::parser

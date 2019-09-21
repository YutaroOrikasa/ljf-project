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

template <template <typename...> class class_template, typename T>
constexpr bool is_class_template_instance_v_impl = false;

template <template <typename...> class class_template, typename... Ts>
constexpr bool is_class_template_instance_v_impl<class_template, class_template<Ts...>> = true;

template <template <typename...> class class_template, typename T>
constexpr bool is_class_template_instance_v = is_class_template_instance_v_impl<class_template, std::decay_t<T>>;

static_assert(is_class_template_instance_v<std::vector, std::vector<int>>);
static_assert(!is_class_template_instance_v<std::tuple, std::vector<int>>);

template <typename T>
constexpr bool is_variant_v = is_class_template_instance_v<std::variant, T>;

template <typename T>
constexpr bool is_tuple_v = is_class_template_instance_v<std::tuple, T>;

static_assert(is_tuple_v<std::tuple<int>>);
static_assert(!is_tuple_v<int>);

template <typename T, typename Tuple, typename... Args>
constexpr T make_from_tuple_and_args(Tuple &&tuple, Args &&... args)
{
    return std::make_from_tuple<T>(
        std::tuple_cat(
            std::forward<Tuple>(tuple),
            std::tuple(std::forward<Args>(args)...)));
}

// These wrapper functions make_from_tuple() exist to show human readable compile error message
// when type error occurred.
template <typename Ret, typename... Ts>
Ret make_from_tuple(std::tuple<Ts...> &&t)
{
    static_assert(std::is_constructible_v<Ret, Ts...>);
    return std::make_from_tuple<Ret>(std::move(t));
}
template <typename Ret, typename... Ts>
Ret make_from_tuple(std::tuple<Ts...> &t)
{
    static_assert(std::is_constructible_v<Ret, Ts...>);
    return std::make_from_tuple<Ret>(t);
}

template <typename Ret>
struct MakeFromVariantVisitor
{
    template <typename T>
    Ret operator()(T &&t) const
    {
        if constexpr (is_variant_v<T>)
        {
            return std::visit(*this, std::forward<T>(t));
        }
        else if constexpr (is_tuple_v<T>)
        {
            return detail::make_from_tuple<Ret>(std::forward<T>(t));
        }
        else
        {
            // This static_assert is for printing human readable compile error message
            // when type error occurred.
            static_assert(std::is_constructible_v<Ret, T>);
            return Ret(std::forward<T>(t));
        }
    }
};
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
namespace impl
{
template <typename... Tuples>
auto tuple_cat(Tuples &&... t)
{
    using result_tuple_ty = decltype(std::tuple_cat(std::forward<Tuples>(t)...));
    // workaround for template constructor that accept 1 argument.
    // Making tuple size of the restult of std::tuple_cat >= 2
    // by adding Dummy object.
    // In std::tuple_cat, codes that has same semantics executed:
    //   std::tuple<T&&> t0 =...;
    //   std::tuple<T> t = std::move(t0); // (A)
    // Normally statement (A) calles T(T&&) constructor
    // in template<class _Tuple> tuple(_Tuple&&) constructor,
    // but if std::is_constructible_v<T, std::tuple<T&&> > is true,
    // no constructor of tuple selected.
    // This behavior does not occur tuple size of std::tuple_cat(...) >= 2.
    if constexpr (std::tuple_size<result_tuple_ty>::value == 1)
    {
        struct Dummy
        {
        };

        auto tpl = std::tuple_cat(std::tuple<Dummy>(), std::forward<Tuples>(t)...);
        using tpl_elem1_ty = std::tuple_element_t<1, decltype(tpl)>;
        return std::make_tuple(std::forward<tpl_elem1_ty>(std::get<1>(tpl)));
    }
    else
    {
        return std::tuple_cat(std::forward<Tuples>(t)...);
    }
}
// template <typename Tuples>
// auto tuple_cat(Tuples && t)
// {
//     return std::forward<Tuples>(t);
// }
} // namespace impl

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
        return std::tuple<std::decay_t<T>>(std::forward<T>(t));
    }
}
// return type: std::tuple<...>
template <typename... Ts>
auto discard_separator(Ts &&... ts)
{
    return impl::tuple_cat(
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

template <typename Ret>
inline constexpr auto make_from_variant = [](auto &&variant) {
    return detail::MakeFromVariantVisitor<Ret>()(
        std::forward<decltype(variant)>(variant));
};

/// F should be deduced by constructor argument, void is dummy.
template <typename TResult = void, typename F = void, bool convert_from_variant_tuple = true>
class Parser
{
    F f_;
    using this_type = Parser<TResult, F>;

public:
    constexpr Parser() = default;
    /*implicit*/ constexpr Parser(F &&f) : f_(std::move(f)) {}
    /*implicit*/ constexpr Parser(const F &f) : f_(f) {}

    template <typename TokenStream>
    auto operator()(TokenStream &&ts) const
    {
        static_assert(!std::is_void_v<decltype(f_(std::forward<TokenStream>(ts)))>,
                      "parser function must return non void type");

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
                if constexpr (convert_from_variant_tuple)
                {
                    return Result(make_from_variant<TResult>(result.extract_success()));
                }
                else
                {
                    return Result(TResult(result.extract_success()));
                }
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

    constexpr auto copy_func() const
    {
        return f_;
    }

    /*** for PlaceHolder ***/

    template <typename TResult_, class TokenStream>
    friend class PlaceHolder;

    constexpr auto set_func(const F &f)
    {
        return f_ = f;
    }

    constexpr const F &func() const
    {
        return f_;
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
    constexpr auto operator<<=(const Parser<R0, F> &p) const
    {
        return Parser<R, F>(p.copy_func());
    }

    template <typename F>
    constexpr auto operator<<=(const F &f) const
    {
        return Parser<R, F>(f);
    }
};

template <typename T>
inline constexpr auto result_type = ResultType<T>();

template <typename F>
class Converter
{
private:
    F conv_;

public:
    explicit constexpr Converter(const F &conv) : conv_(conv) {}

    template <typename P>
    constexpr auto operator<<=(const P &parser) const
    {
        return Parser([=, *this](auto &&token_stream) {
            return conv_(parser(token_stream));
        });
    }
};

// usage: Parser p2 = converter(function_object) <<= p0
// p2's result.success() will be function_object(p0(stream).success())
// if p0 result is success.
template <typename F>
constexpr auto converter(F &&f)
{
    return Converter(
        [conv = std::forward<F>(f)](auto &&result) {
            using ConvertedContent = decltype(conv(result.extract_success()));
            if (result.failed())
            {
                return Result<ConvertedContent>(result.extract_error_ptr());
            }
            return Result<ConvertedContent>(conv(result.extract_success()));
        });
}

template <typename TResult, class TokenStream>
class PlaceHolder
{
private:
    struct NonPropagateBool;
    NonPropagateBool lazy_init_check_;
    std::string name_;
    using func_type = std::function<Result<TResult>(TokenStream &)>;
    using parser_type = Parser<TResult, func_type>;
    std::shared_ptr<parser_type> parser_sptr_ = std::make_shared<parser_type>();

public:
    PlaceHolder() : lazy_init_check_(true)
    {
        // std::cerr << " PlaceHolder() " << this << "\n";
    }
    explicit PlaceHolder(const char *name) : lazy_init_check_(true), name_(name)
    {
        // std::cerr << " PlaceHolder() " << this << "\n";
    }
    ~PlaceHolder()
    {
        if (lazy_init_check_)
        {
            if (!has_parser())
            {
                std::cerr << "~PlaceHolder() name=" << name_ << " at " << this << " not initialized!\n";
            }

            assert(has_parser());
        }
    }
    template <typename UResult, typename F>
    PlaceHolder &operator=(const Parser<UResult, F> &parser)
    {
        parser_sptr_->set_func(func_type(result_type<TResult> <<= parser));
        return *this;
    }

    template <typename F>
    PlaceHolder &operator=(const F &parser)
    {
        parser_sptr_->set_func(func_type(result_type<TResult> <<= parser));
        return *this;
    }

    auto operator()(TokenStream &ts) const
    {
        if (!has_parser())
        {
            std::cerr << "PlaceHolder name=" << name_ << " at " << this << " not initialized!\n";
        }

        assert(has_parser() && "no parsers assigned");
        return (*parser_sptr_)(ts);
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

    bool has_parser() const noexcept
    {
        return bool(parser_sptr_->func());
    }

private:
    struct NonPropagateBool
    {
        bool value = false;
        /*implicit*/ NonPropagateBool(bool b) : value(b){};
        NonPropagateBool(const NonPropagateBool &){};
        NonPropagateBool &operator=(const NonPropagateBool &)
        {
            return *this;
        };
        explicit operator bool() const noexcept
        {
            return value;
        }
    };
};

} // namespace ljf::python::parser

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <cassert>

// helpers
namespace ljf::python::parser::detail {

template <template <typename...> class class_template, typename T>
constexpr bool is_class_template_instance_v_impl = false;

template <template <typename...> class class_template, typename... Ts>
constexpr bool
    is_class_template_instance_v_impl<class_template, class_template<Ts...>> =
        true;

template <template <typename...> class class_template, typename T>
constexpr bool is_class_template_instance_v =
    is_class_template_instance_v_impl<class_template, std::decay_t<T>>;

static_assert(is_class_template_instance_v<std::vector, std::vector<int>>);
static_assert(!is_class_template_instance_v<std::tuple, std::vector<int>>);

template <typename T>
constexpr bool is_variant_v = is_class_template_instance_v<std::variant, T>;

template <typename T>
constexpr bool is_tuple_v = is_class_template_instance_v<std::tuple, T>;

template <typename T>
constexpr bool is_optional_v = is_class_template_instance_v<std::optional, T>;

static_assert(is_tuple_v<std::tuple<int>>);
static_assert(!is_tuple_v<int>);

template <typename T, typename Tuple, typename... Args>
constexpr T make_from_tuple_and_args(Tuple &&tuple, Args &&...args) {
    return std::make_from_tuple<T>(std::tuple_cat(
        std::forward<Tuple>(tuple), std::tuple(std::forward<Args>(args)...)));
}

struct ApplyToVariantOptionalTupleVisitor {
    template <typename F, typename T> auto operator()(F &&f, T &&t) const {
        if constexpr (is_variant_v<T>) {
            // mutable is necessary for forward f
            auto visitor = [*this, f = std::forward<F>(f)](auto &&arg) mutable {
                return (*this)(std::forward<F>(f),
                               std::forward<decltype(arg)>(arg));
            };
            return std::visit(std::move(visitor), std::forward<T>(t));
        } else if constexpr (is_tuple_v<T>) {
            return std::apply(std::forward<F>(f), std::forward<T>(t));
        } else if constexpr (is_optional_v<T>) {
            if (!t) {
                return std::forward<F>(f)();
            }

            return (*this)(std::forward<F>(f), *std::forward<T>(t));
        } else {
            // This static_assert is for printing human readable compile
            // error message when type error occurred.
            static_assert(std::is_invocable_v<F, T>);
            return std::forward<F>(f)(std::forward<T>(t));
        }
    }
};

inline constexpr ApplyToVariantOptionalTupleVisitor
    apply_to_variant_optional_tuple;

// This wrapper function exists only for printing human readable compile
// error message when type error occurred.
template <typename Ret, typename... Args> Ret construct_impl(Args &&...args) {
    static_assert(std::is_constructible_v<Ret, Args...>);
    return Ret(std::forward<Args>(args)...);
}

template <typename Ret>
inline constexpr auto construct = [](auto &&...args) {
    return construct_impl<Ret>(std::forward<decltype(args)>(args)...);
};

} // namespace ljf::python::parser::detail

namespace ljf::python::parser {

using detail::apply_to_variant_optional_tuple;

// alias for compatibility
inline constexpr auto apply_variant_tuple = apply_to_variant_optional_tuple;

template <typename... P> class Sequence;

/// ordered choice
template <typename... P> class Choice;

struct Separator {
    template <typename... Args> explicit Separator(Args...) {}
};

namespace detail {
    namespace impl {

        // Returns tuple's content.
        // If T is reference (eg. Type&, Type&&),
        // return type is also reference.
        // Reference qualification is saved at return type.
        template <typename T> T tuple_strip(std::tuple<T> &&tuple) {
            return std::get<0>(std::move(tuple));
        }

        static_assert(
            std::is_same_v<
                int, decltype(tuple_strip(std::declval<std::tuple<int>>()))>);
        static_assert(
            std::is_same_v<int &, decltype(tuple_strip(
                                      std::declval<std::tuple<int &>>()))>);
        static_assert(
            std::is_same_v<int &&, decltype(tuple_strip(
                                       std::declval<std::tuple<int &&>>()))>);

        template <typename... Tuples>
        decltype(auto) tuple_cat_and_strip(Tuples &&...t) {
            using result_tuple_ty =
                decltype(std::tuple_cat(std::forward<Tuples>(t)...));

            if constexpr (std::tuple_size<result_tuple_ty>::value == 1) {
                auto tpl = std::tuple_cat(std::forward<Tuples>(t)...);

                // Return type is not tuple, just T.
                // tuple_cat_and_strip() strips tuple that size is 1.
                // tuple_cat_and_strip() returns T instead of std::tuple<T>.
                // And save reference qualification of tpl[0],
                // we use tuple_strip() instead of std::get<0>(tpl)
                return tuple_strip(std::move(tpl));
            } else {
                return std::tuple_cat(std::forward<Tuples>(t)...);
            }
        }

        static_assert(
            std::is_same_v<int, decltype(tuple_cat_and_strip(
                                    std::declval<std::tuple<int>>()))>);
        static_assert(
            std::is_same_v<int &, decltype(tuple_cat_and_strip(
                                      std::declval<std::tuple<int &>>()))>);
        static_assert(
            std::is_same_v<int &&, decltype(tuple_cat_and_strip(
                                       std::declval<std::tuple<int &&>>()))>);
        static_assert(std::is_same_v<
                      std::tuple<int, int &, int &&>,
                      decltype(tuple_cat_and_strip(
                          std::declval<std::tuple<int, int &, int &&>>()))>);
        static_assert(std::is_same_v<std::tuple<int, int &, int &&>,
                                     decltype(tuple_cat_and_strip(
                                         std::declval<std::tuple<int>>(),
                                         std::declval<std::tuple<int &>>(),
                                         std::declval<std::tuple<int &&>>()))>);

    } // namespace impl

    // wrap T with std::tuple
    // but if T is Separator, return empty tuple
    template <typename T> auto to_tuple(T &&t) {
        if constexpr (std::is_same_v<std::decay_t<T>, Separator>) {
            return std::tuple();
        } else {
            return std::tuple<std::decay_t<T>>(std::forward<T>(t));
        }
    }
    // return type: std::tuple<T0, T1, ...>
    // or
    // return type: T0
    // This function will not return std::tuple<T0>.
    template <typename... Ts> auto discard_separator(Ts &&...ts) {
        return impl::tuple_cat_and_strip(to_tuple(std::forward<Ts>(ts))...);
    }

} // namespace detail

class Error {
private:
    // The token that caused parsing error.
    Token token_;
    std::string msg_;

public:
    // token: The token that caused parsing error.
    Error(const Token &token, const std::string &msg)
        : token_(token), msg_(msg) {}

    // token: The token that caused parsing error.
    template <typename... Msgs>
    Error(const Token &token, const std::string &msg, Msgs &&...msgs)
        : token_(token), msg_((msg + ... + msgs)) {}

    // token: The token that caused parsing error.
    explicit Error(const Token &token) : Error(token, ""){};

    virtual ~Error() = default;

    // return: The token that caused parsing error.
    virtual const Token &token() const { return token_; }

    virtual const std::string &str() const { return msg_; }

    template <typename... Msgs>
    std::unique_ptr<Error> copy_ptr_with_new_msg(const std::string &msg,
                                                 Msgs &&...msgs) {
        return std::make_unique<Error>(Error(token_, msg, msgs...));
    }
};

template <typename Out> Out &operator<<(Out &out, const Error &e) {
    out << "Error: ";
    if (!e.str().empty()) {
        out << e.str() << ": ";
    }

    auto &token = e.token();
    if (token.is_eof()) {
        out << "error token = <EOF>";
    } else if (token.is_newline()) {
        out << "error token = <NEWLINE>";
    } else if (token.is_indent()) {
        out << "error token = <INDENT>";
    } else if (token.is_dedent()) {
        out << "error token = <DEDENT>";
    } else if (token.is_invalid()) {
        out << "invalid token `" << token.str() << "`: ";
        out << token.error_message();
    } else {
        out << "error token = `" << token.str() << "`";
    }
    out << "\n";

    auto &loc = token.source_location();
    out << loc << ":\n";
    out << "    " << loc.line() << "\n";
    out << "    ";
    for (size_t i = 0; i < loc.column() - 1; i++) {
        out << " ";
    }
    out << "^";

    return out;
}

template <typename T> struct Success {
    T value;
    explicit Success(T &&t) : value(std::move(t)) {}
    explicit Success(T &t) : value(t) {}
};

template <typename T> auto success_move(T &&t) { return Success(std::move(t)); }

template <typename T> class Result {
private:
    std::variant<T, std::unique_ptr<Error>> value_;

public:
    using success_type = T;
    explicit Result(T &&t) : value_(std::move(t)) {}
    explicit Result(const T &t) : value_(t) {}
    /*implicit*/ Result(std::unique_ptr<Error> e) : value_(std::move(e)) {}

    /*implicit*/ Result(Success<T> &&suc) : value_(std::move(suc.value)) {}

    template <typename U>
    /*implicit*/ Result(Success<U> &&suc) : value_(std::move(suc.value)) {}

    bool failed() const noexcept { return !std::holds_alternative<T>(value_); }

    explicit operator bool() const noexcept { return !failed(); }

    const T &success() const &noexcept {
        assert(!failed());
        return std::get<T>(value_);
    }

    T success() && {
        assert(!failed());
        return std::move(std::get<T>(value_));
    }

    T extract_success() {
        assert(!failed());
        return std::move(std::get<T>(value_));
    }

    const Error &error() const noexcept {
        assert(failed());
        return *std::get<std::unique_ptr<Error>>(value_);
    }

    std::unique_ptr<Error> error_ptr() && {
        assert(failed());
        return std::move(std::get<std::unique_ptr<Error>>(value_));
    }

    std::unique_ptr<Error> extract_error_ptr() {
        assert(failed());
        return std::move(std::get<std::unique_ptr<Error>>(value_));
    }

    template <size_t I> const auto &get() const {
        return std::get<I>(success());
    }
    template <typename F> auto visit(F &&visitor) const {
        return visit_impl(std::forward<F>(visitor), success());
    }

private:
    template <typename F, typename U>
    static auto visit_impl(F &&visitor, const U &t) {
        return std::forward<F>(visitor)(t);
    }

    template <typename F, typename... Ts>
    static auto visit_impl(F &&visitor, const std::variant<Ts...> &t) {
        return std::visit(std::forward<F>(visitor), t);
    }
};

template <size_t I, typename T> const auto &get(const Result<T> &result) {
    return std::get<I>(result.success());
}

template <typename... Args> auto make_error(Args &&...args) {
    return std::make_unique<Error>(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
Result<T> make_error_result(Args &&...args) {
    return Result<T>(make_error(std::forward<Args>(args)...));
}

template <typename ToResultContent, typename FromResultType>
Result<ToResultContent>
move_to_another_error_result(FromResultType &&from_result) {
    assert(from_result.failed());
    return Result<ToResultContent>(from_result.extract_error_ptr());
}

template <typename Ret>
inline constexpr auto make_from_variant = [](auto &&variant) {
    return apply_variant_tuple(detail::construct<Ret>,
                               std::forward<decltype(variant)>(variant));
};

/// F should be deduced by constructor argument, void is dummy.
template <typename TResult = void, typename F = void,
          bool convert_from_variant_tuple = true>
class Parser {
    F f_;
    using this_type = Parser<TResult, F>;

public:
    constexpr Parser() = default;
    /*implicit*/ constexpr Parser(F &&f) : f_(std::move(f)) {}
    /*implicit*/ constexpr Parser(const F &f) : f_(f) {}

    template <typename TokenStream> auto operator()(TokenStream &&ts) const {
        static_assert(
            !std::is_void_v<decltype(f_(std::forward<TokenStream>(ts)))>,
            "parser function must return non void type");

        // We don't create Result<Result<T>>, so use to_result() helper.
        // Type of result is flatten.
        Result result = to_result(f_(std::forward<TokenStream>(ts)));
        if constexpr (std::is_same_v<TResult, void>) {

            return Result(std::move(result));
        } else {
            if (result) {
                if constexpr (convert_from_variant_tuple) {
                    return Result(
                        make_from_variant<TResult>(result.extract_success()));
                } else {
                    return Result(TResult(result.extract_success()));
                }
            } else {
                return Result<TResult>(result.extract_error_ptr());
            }
        }
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const {
        return Sequence<this_type, Parser2>(*this, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const {
        return Choice<this_type, Parser2>(*this, parser2);
    }

private:
    /*** to_result() functions ***/

    template <typename T> static auto to_result(T &&t) {
        return Result<T>(std::move(t));
    }

    template <typename T> static auto to_result(Result<T> result) {
        return std::move(result);
    }

    template <typename T> static auto to_result(T &t) = delete;

    /*** for ResultType<T> ***/

    template <typename T> friend struct ResultType;

    constexpr auto copy_func() const { return f_; }

    /*** for PlaceHolder ***/

    template <typename TResult_, class TokenStream> friend class PlaceHolder;

    constexpr auto set_func(const F &f) { return f_ = f; }

    constexpr const F &func() const { return f_; }
};

template <typename... Ps> class Sequence {
    std::tuple<Ps...> parser_tuple_;
    static constexpr size_t tuple_size_ = sizeof...(Ps);
    using this_type = Sequence<Ps...>;

public:
    constexpr Sequence(const Ps &...parsers) : parser_tuple_(parsers...) {}

    template <typename TokenStream>
    auto operator()(TokenStream &token_stream) const {
        return parse<0>(token_stream);
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const {
        // make Sequence<Ps..., Parser2> from this and parser2
        // We don't make nested sequence type such like
        // `Sequence<Sequence<Ps...>, Parser2>`
        return detail::make_from_tuple_and_args<Sequence<Ps..., Parser2>>(
            parser_tuple_, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const {
        return Choice<this_type, Parser2>(*this, parser2);
    }

private:
    // return type: Result<std::tuple<...>>
    template <size_t I, typename TokenStream, typename... Results>
    auto parse(TokenStream &token_stream, Results &&...results) const {
        if constexpr (I == tuple_size_) {
            return Result(detail::discard_separator(std::move(results)...));
        } else {
            auto result = std::get<I>(parser_tuple_)(token_stream);
            if (result.failed()) {
                using ResultTy =
                    decltype(parse<I + 1>(token_stream, std::move(results)...,
                                          std::move(result).success()));
                // return error
                return ResultTy(std::move(result).error_ptr());
            }

            return parse<I + 1>(token_stream, std::move(results)...,
                                std::move(result).success());
        }
    }
};

template <typename... Ps> class Choice {
    std::tuple<Ps...> parser_tuple_;
    static constexpr size_t tuple_size_ = sizeof...(Ps);
    using this_type = Choice<Ps...>;

public:
    constexpr Choice(const Ps &...parsers) : parser_tuple_(parsers...) {}

    template <typename TokenStream>
    auto operator()(TokenStream &token_stream) const {
        return parse<0>(token_stream);
    }

    template <typename Parser2>
    constexpr auto operator+(const Parser2 &parser2) const {
        return Sequence<this_type, Parser2>(*this, parser2);
    }

    template <typename Parser2>
    constexpr auto operator|(const Parser2 &parser2) const {
        // We don't make nested type Choice<Choice<Ps...>, Parser2>
        // as same as Sequence<?>::operator+().
        return detail::make_from_tuple_and_args<Choice<Ps..., Parser2>>(
            parser_tuple_, parser2);
    }

private:
    // return type: Result<std::variant<...>>
    template <size_t I, typename TokenStream>
    auto parse(TokenStream &token_stream) const {
        using variant_type =
            std::variant<std::decay_t<typename decltype(std::declval<Ps>()(
                token_stream))::success_type>...>;
        using ResultTy = Result<variant_type>;

        static_assert(I < tuple_size_);

        auto current_pos = token_stream.current_position();
        auto result = std::get<I>(parser_tuple_)(token_stream);

        if (!result.failed()) {
            return ResultTy(variant_type(std::in_place_index<I>,
                                         std::move(result).success()));
        }

        // This is LL1 parser.
        // If current_position was changed, it is parse error.
        if (token_stream.current_position() != current_pos) {
            // return error
            return ResultTy(
                std::move(result).error_ptr()->copy_ptr_with_new_msg(
                    "Unexpected token found"));
        } else {
            if constexpr (I + 1 < tuple_size_) {
                // try next parser
                return parse<I + 1>(token_stream);
            } else {
                // There are no parser left.
                // Return the last result (== error)
                return ResultTy(
                    std::move(result).error_ptr()->copy_ptr_with_new_msg(
                        "Unexpected token found"));
            }
        }
    }
};

template <typename Parser, typename TokenStream>
using result_content_t = std::decay_t<
    decltype(std::declval<Parser>()(std::declval<TokenStream &>()).success())>;

template <typename Result, typename Pos, typename TokenStream>
bool LL1_parser_fatally_failed(const Result &result,
                               const Pos &initial_position,
                               const TokenStream &stream) {
    return result.failed() && (initial_position != stream.current_position());
}

template <typename T> struct LL1Result {
    Result<T> result;

private:
    bool fatally_failed_ = false;

public:
    explicit LL1Result(Result<T> &&res, bool fatally_failed)
        : result(std::move(res)), fatally_failed_(fatally_failed) {}

    bool fatally_failed() const noexcept { return fatally_failed_; }
};

template <typename Parser, class TokenStream>
auto LL1_parse(const Parser &p, TokenStream &token_stream) {
    auto init_pos = token_stream.current_position();

    auto result = p(token_stream);
    bool fatally_failed =
        LL1_parser_fatally_failed(result, init_pos, token_stream);

    return LL1Result(std::move(result), fatally_failed);
}

/// return type: Parser<std::vector<?>,?>
template <typename P> constexpr auto many(P &&parser) {
    return Parser([parser](auto &&token_stream) {
        using vec_value_ty = result_content_t<P, decltype(token_stream)>;
        using vec_ty = std::vector<vec_value_ty>;
        vec_ty vec;
        while (true) {
            auto init_pos = token_stream.current_position();

            auto result = parser(token_stream);
            if (LL1_parser_fatally_failed(result, init_pos, token_stream)) {
                return Result<vec_ty>(result.extract_error_ptr());
            }

            // parser not matched token and only lookahead was done.
            // finish many()
            if (result.failed()) {
                break;
            }

            vec.push_back(result.extract_success());
        }
        return Result<vec_ty>(std::move(vec));
    });
}

template <typename R> struct ResultType {
    template <typename F>
    constexpr auto operator<<=(const Parser<void, F> &p) const {
        return Parser<R, F>(p.copy_func());
    }

    template <typename F> constexpr auto operator<<=(const F &f) const {
        return Parser<R, F>(f);
    }
};

template <typename T> inline constexpr auto result_type = ResultType<T>();

template <typename F> class Converter {
private:
    F conv_;

public:
    explicit constexpr Converter(const F &conv) : conv_(conv) {}

    template <typename P> constexpr auto operator<<=(const P &parser) const {
        return Parser([=, *this](auto &&token_stream) {
            return conv_(parser(token_stream));
        });
    }
};

template <typename T>
constexpr bool is_result_v = detail::is_class_template_instance_v<Result, T>;

/// This function automatically wraps returned type of F() with Result.
/// never makes nested Result<>
template <typename F, typename R>
auto apply_to_result(const F &func, R &&result) {
    using ReturnedType =
        std::decay_t<decltype(func(std::forward<R>(result).success()))>;

    using ResultType = std::conditional_t<is_result_v<ReturnedType>,
                                          ReturnedType, Result<ReturnedType>>;

    if (result.failed()) {
        return ResultType(std::forward<R>(result).error_ptr());
    }
    return ResultType(func(std::forward<R>(result).success()));
}

// usage: Parser p2 = converter(function_object) <<= p0
// p2's result.success() will be function_object(p0(stream).success())
// if p0 result is success.
template <typename F> constexpr auto converter(const F &f) {
    auto conv = [f](auto &&success) {
        return apply_variant_tuple(f, std::forward<decltype(success)>(success));
    };
    return Converter([conv = std::move(conv)](auto &&result) {
        return apply_to_result(std::move(conv),
                               std::forward<decltype(result)>(result));
    });
}

// same as converter, but
// don't strip variant, optional and tuple
template <typename F> constexpr auto converter_no_strip(F &&f) {
    return Converter([conv = std::forward<F>(f)](auto &&result) {
        using ConvertedContent = decltype(conv(result.extract_success()));
        if (result.failed()) {
            return Result<ConvertedContent>(result.extract_error_ptr());
        }
        return Result<ConvertedContent>(conv(result.extract_success()));
    });
}

namespace detail {
    template <typename T> struct BraceInitialized {
        T value;
        template <typename... Args>
        explicit BraceInitialized(Args &&...args)
            : value{std::forward<Args>(args)...} {}
    };
    template <typename Tuple> struct BraceInitArgs {
        Tuple arg_tuple;
        explicit BraceInitArgs(Tuple &&tpl) : arg_tuple(std::move(tpl)) {}
        template <typename T>
        /*implicit*/ operator T() && {
            return std::move(
                make_from_variant<BraceInitialized<T>>(arg_tuple).value);
        }
    };

    inline constexpr auto brace_init_fn = [](auto &&...args) {
        return BraceInitArgs(
            std::make_tuple(std::forward<decltype(args)>(args)...));
    };
} // namespace detail

inline constexpr auto brace_init = converter(detail::brace_init_fn);

template <typename TResult, class TokenStream> class PlaceHolder {
private:
    struct NonPropagateBool;
    NonPropagateBool lazy_init_check_;
    std::string name_;
    using func_type = std::function<Result<TResult>(TokenStream &)>;
    using parser_type = Parser<TResult, func_type>;
    std::shared_ptr<parser_type> parser_sptr_ = std::make_shared<parser_type>();

public:
    PlaceHolder() : lazy_init_check_(true) {
        // std::cerr << " PlaceHolder() " << this << "\n";
    }
    explicit PlaceHolder(const char *name)
        : lazy_init_check_(true), name_(name) {
        // std::cerr << " PlaceHolder() " << this << "\n";
    }
    ~PlaceHolder() {
        if (lazy_init_check_) {
            if (!has_parser()) {
                std::cerr << "~PlaceHolder() name=" << name_ << " at " << this
                          << " not initialized!\n";
            }

            assert(has_parser());
        }
    }
    template <typename UResult, typename F>
    PlaceHolder &operator=(const Parser<UResult, F> &parser) {
        parser_sptr_->set_func(func_type(result_type<TResult> <<= parser));
        return *this;
    }

    template <typename F> PlaceHolder &operator=(const F &parser) {
        parser_sptr_->set_func(func_type(result_type<TResult> <<= parser));
        return *this;
    }

    PlaceHolder &operator=(const PlaceHolder &parser) {
        parser_sptr_->set_func(func_type(result_type<TResult> <<= parser));
        return *this;
    }

    auto operator()(TokenStream &ts) const {
        if (!has_parser()) {
            std::cerr << "PlaceHolder name=" << name_ << " at " << this
                      << " not initialized!\n";
        }

        assert(has_parser() && "no parsers assigned");
        return (*parser_sptr_)(ts);
    }

    template <typename Parser2> auto operator+(const Parser2 &parser2) const {
        return Sequence<PlaceHolder, Parser2>(*this, parser2);
    }

    template <typename Parser2> auto operator|(const Parser2 &parser2) const {
        return Choice<PlaceHolder, Parser2>(*this, parser2);
    }

    bool has_parser() const noexcept { return bool(parser_sptr_->func()); }

private:
    struct NonPropagateBool {
        bool value = false;
        /*implicit*/ NonPropagateBool(bool b) : value(b){};
        NonPropagateBool(const NonPropagateBool &){};
        NonPropagateBool &operator=(const NonPropagateBool &) { return *this; };
        explicit operator bool() const noexcept { return value; }
    };
};

} // namespace ljf::python::parser

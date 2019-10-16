#include "common.hpp"

#include "ljf-python/grammar/stmt.hpp"

#include <tuple>
namespace ljf::python::grammar
{
namespace detail
{

template <typename T>
struct LL1Result
{
    Result<T> result;

private:
    bool fatally_failed_ = false;

public:
    explicit LL1Result(Result<T> &&res, bool fatally_failed)
        : result(std::move(res)),
          fatally_failed_(fatally_failed) {}

    bool fatally_failed() const noexcept
    {
        return fatally_failed_;
    }
};

template <typename Parser, class TokenStream>
auto LL1_parse(const Parser &p, TokenStream &token_stream)
{
    auto init_pos = token_stream.current_position();

    auto result = p(token_stream);
    bool fatally_failed = LL1_parser_failed(result, init_pos, token_stream);

    return LL1Result(std::move(result), fatally_failed);
}

/// parse `p sep ... sep p [sep [end]]`
/// equivalent to many(p + sep) + [p | end]
/// result type: std::tuple{vector<result of p>, bool:ends_with_sep, std::optional<result of end>}
template <typename Parser, typename SepParser, typename EndParser>
constexpr auto sep_many_optsep_optend(Parser p, SepParser sep, EndParser end)
{

    return parser::Parser(
        [=](auto &&token_stream) {
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
} // namespace ljf::python::grammar

using grammar::detail::sep_many_optsep_optend;

TEST(SepManyOptsepOptend, ZeroElement)
{
    constexpr auto input = "";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(0, vec.size());
    ASSERT_FALSE(ends_with_sep);
    ASSERT_FALSE(opt_end);
}

TEST(SepManyOptsepOptend, ZeroElementEndsWithEnd)
{
    constexpr auto input = "*";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(0, vec.size());
    ASSERT_FALSE(ends_with_sep);
    ASSERT_TRUE(opt_end);
}

TEST(SepManyOptsepOptend, OneElement)
{
    constexpr auto input = "a";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_FALSE(ends_with_sep);
    ASSERT_FALSE(opt_end);
}

TEST(SepManyOptsepOptend, OneElementEndsWithSep)
{
    constexpr auto input = "a,";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_TRUE(ends_with_sep);
    ASSERT_FALSE(opt_end);
}

TEST(SepManyOptsepOptend, OneElementEndsWithSepEnd)
{
    constexpr auto input = "a,*";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_TRUE(ends_with_sep);
    ASSERT_TRUE(opt_end);
}

TEST(SepManyOptsepOptend, TwoElements)
{
    constexpr auto input = "a, b";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_FALSE(ends_with_sep);
    ASSERT_FALSE(opt_end);
}

TEST(SepManyOptsepOptend, TwoElementsEndsWithSep)
{
    constexpr auto input = "a, b,";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_TRUE(ends_with_sep);
    ASSERT_FALSE(opt_end);
}

TEST(SepManyOptsepOptend, TwoElementsEndsWithSepEnd)
{
    constexpr auto input = "a, b, *";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep, opt_end] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_TRUE(ends_with_sep);
    ASSERT_TRUE(opt_end);
}

TEST(SepManyOptsepOptendInvalid, NoSep)
{
    constexpr auto input = "a b";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_FALSE(result) << result.error();
}

TEST(SepManyOptsepOptendInvalid, EndWithoutSep)
{
    constexpr auto input = "a, b *";
    auto result = parse_until_end(sep_many_optsep_optend(parser::identifier, ","_sep, "*"_p), input);
    ASSERT_FALSE(result) << result.error();
}

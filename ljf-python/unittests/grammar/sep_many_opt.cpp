#include "common.hpp"

#include "ljf-python/grammar/stmt.hpp"
namespace ljf::python::grammar
{
namespace detail
{
/// equivalent to optional(p + many(sep + p) + optional(sep))
template <typename Parser, typename SepParser>
constexpr auto sep_many_opt(Parser p, SepParser sep)
{
    auto opt_end = [](auto sep) {
        using parser::Result;
        return parser::Parser([sep](auto &token_stream) {
            auto init_pos = token_stream.current_position();
            auto result = sep(token_stream);
            if (LL1_parser_failed(result, init_pos, token_stream))
            {
                return Result<bool>(result.extract_error_ptr());
            }

            // parser not matched token and only lookahead was done.
            // finish opt_end()
            if (result.failed())
            {
                return Result<bool>(false);
            }

            return Result<bool>(true);
        });
    };
    auto parser = p + opt_end(sep);

    return parser::Parser(
        [parser](auto &&token_stream) {
            using namespace parser;

            using vec_value_ty = result_content_t<Parser, decltype(token_stream)>;
            using vec_ty = std::vector<vec_value_ty>;
            using pair_ty = std::pair<vec_ty, bool>;
            using ResultTy = Result<pair_ty>;

            vec_ty vec;

            while (true)
            {
                auto init_pos = token_stream.current_position();

                auto result = parser(token_stream);
                if (LL1_parser_failed(result, init_pos, token_stream))
                {
                    return ResultTy(result.extract_error_ptr());
                }

                // parser not matched token and only lookahead was done.
                // finish sep_many_opt()
                if (result.failed())
                {
                    return ResultTy(pair_ty{std::move(vec), false});
                }
                auto [parsed, has_sep] = result.extract_success();
                vec.push_back(std::move(parsed));
                if (has_sep)
                {
                    return ResultTy(pair_ty{std::move(vec), true});
                }
            }
        });
}
} // namespace detail
} // namespace ljf::python::grammar

using grammar::detail::sep_many_opt;

TEST(SepManyOpt, ZeroElement)
{
    constexpr auto input = "";
    auto result = parse_until_end(sep_many_opt(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(0, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOpt, OneElement)
{
    constexpr auto input = "a";
    auto result = parse_until_end(sep_many_opt(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOpt, OneElementEndsWithSep)
{
    constexpr auto input = "a,";
    auto result = parse_until_end(sep_many_opt(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_TRUE(ends_with_sep);

}

TEST(SepManyOpt, TwoElements)
{
    constexpr auto input = "a, b";
    auto result = parse_until_end(sep_many_opt(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOpt, TwoElementsEndsWithSep)
{
    constexpr auto input = "a, b,";
    auto result = parse_until_end(sep_many_opt(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_TRUE(ends_with_sep);
}

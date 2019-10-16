#include "common.hpp"

#include "ljf-python/grammar/stmt.hpp"


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

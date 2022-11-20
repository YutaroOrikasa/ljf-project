#include "common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using grammar::detail::sep_many_optsep;

TEST(SepManyOptsep, ZeroElement) {
    constexpr auto input = "";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(0, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOptsep, OneElement) {
    constexpr auto input = "a";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOptsep, OneElementEndsWithSep) {
    constexpr auto input = "a,";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(1, vec.size());
    ASSERT_TRUE(ends_with_sep);
}

TEST(SepManyOptsep, TwoElements) {
    constexpr auto input = "a, b";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_FALSE(ends_with_sep);
}

TEST(SepManyOptsep, TwoElementsEndsWithSep) {
    constexpr auto input = "a, b,";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_TRUE(result) << result.error();

    auto [vec, ends_with_sep] = result.extract_success();
    EXPECT_EQ(2, vec.size());
    ASSERT_TRUE(ends_with_sep);
}

TEST(SepManyOptsep, Invalid) {
    constexpr auto input = "a b";
    auto result =
        parse_until_end(sep_many_optsep(parser::identifier, ","_sep), input);
    ASSERT_FALSE(result) << result.error();
}

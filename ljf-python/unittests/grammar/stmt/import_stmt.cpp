#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(ImportStmt, one_name)
{
    constexpr auto input = R"(
import a
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    ASSERT_EQ("a", result.success().import_as_names.at(0).names.at(0).name());

    ASSERT_FALSE(result.success().import_as_names.at(0).opt_as_name);
}

TEST(ImportStmt, dotted_name)
{
    constexpr auto input = R"(
import a.b
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    ASSERT_EQ("a", result.success().import_as_names.at(0).names.at(0).name());
    ASSERT_EQ("b", result.success().import_as_names.at(0).names.at(1).name());

    ASSERT_FALSE(result.success().import_as_names.at(0).opt_as_name);
}

TEST(ImportStmt, dotted_name_as)
{
    constexpr auto input = R"(
import a.b as c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    ASSERT_EQ("a", result.success().import_as_names.at(0).names.at(0).name());
    ASSERT_EQ("b", result.success().import_as_names.at(0).names.at(1).name());

    ASSERT_TRUE(result.success().import_as_names.at(0).opt_as_name);

    ASSERT_EQ("c", result.success().import_as_names.at(0).opt_as_name.value().name());
}

TEST(ImportStmt, multiple_dotted_name_as)
{
    constexpr auto input = R"(
import a.b as c, x.y as z
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    ASSERT_EQ("x", result.success().import_as_names.at(1).names.at(0).name());
    ASSERT_EQ("y", result.success().import_as_names.at(1).names.at(1).name());

    ASSERT_TRUE(result.success().import_as_names.at(1).opt_as_name);

    ASSERT_EQ("z", result.success().import_as_names.at(1).opt_as_name.value().name());
}

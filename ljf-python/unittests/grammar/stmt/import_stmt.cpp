#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(ImportStmt, one_name)
{
    constexpr auto input = R"(
import a
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import = std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());

    ASSERT_FALSE(import.opt_as_name);
}

TEST(ImportStmt, dotted_name)
{
    constexpr auto input = R"(
import a.b
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import = std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());
    ASSERT_EQ("b", import.names.at(1).name());

    ASSERT_FALSE(import.opt_as_name);
}

TEST(ImportStmt, dotted_name_as)
{
    constexpr auto input = R"(
import a.b as c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import = std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());
    ASSERT_EQ("b", import.names.at(1).name());

    ASSERT_TRUE(import.opt_as_name);

    ASSERT_EQ("c", import.opt_as_name.value().name());
}

TEST(ImportStmt, multiple_dotted_name_as)
{
    constexpr auto input = R"(
import a.b as c, x.y as z
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import2 = std::get<DottedAsNameVec>(result.success().import_or_import_from).at(1);

    ASSERT_EQ("x", import2.names.at(0).name());
    ASSERT_EQ("y", import2.names.at(1).name());

    ASSERT_TRUE(import2.opt_as_name);

    ASSERT_EQ("z", import2.opt_as_name.value().name());
}

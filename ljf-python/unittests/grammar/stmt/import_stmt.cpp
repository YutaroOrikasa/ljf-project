#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(ImportStmt, one_name) {
    constexpr auto input = R"(
import a
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import =
        std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());

    ASSERT_FALSE(import.opt_as_name);
}

TEST(ImportStmt, dotted_name) {
    constexpr auto input = R"(
import a.b
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import =
        std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());
    ASSERT_EQ("b", import.names.at(1).name());

    ASSERT_FALSE(import.opt_as_name);
}

TEST(ImportStmt, dotted_name_as) {
    constexpr auto input = R"(
import a.b as c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import =
        std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

    ASSERT_EQ("a", import.names.at(0).name());
    ASSERT_EQ("b", import.names.at(1).name());

    ASSERT_TRUE(import.opt_as_name);

    ASSERT_EQ("c", import.opt_as_name.value().name());
}

TEST(ImportStmt, multiple_dotted_name_as) {
    constexpr auto input = R"(
import a.b as c, x.y as z
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import2 =
        std::get<DottedAsNameVec>(result.success().import_or_import_from).at(1);

    ASSERT_EQ("x", import2.names.at(0).name());
    ASSERT_EQ("y", import2.names.at(1).name());

    ASSERT_TRUE(import2.opt_as_name);

    ASSERT_EQ("z", import2.opt_as_name.value().name());
}

TEST(ImportStmt, ImportFromAbsolute) {
    constexpr auto input = R"(
from a.b import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(0, import_from.dot_num);
    ASSERT_EQ("a", import_from.from_names.at(0).name());
    ASSERT_EQ("b", import_from.from_names.at(1).name());
    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);
    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);
}

TEST(ImportStmt, ImportFromRelative) {
    constexpr auto input = R"(
from .a.b import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(1, import_from.dot_num);
    ASSERT_EQ("a", import_from.from_names.at(0).name());
    ASSERT_EQ("b", import_from.from_names.at(1).name());
    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);
    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);
}

TEST(ImportStmt, ImportFromRelativeMultiple) {
    constexpr auto input = R"(
from .a.b import c, d
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(1, import_from.dot_num);
    ASSERT_EQ("a", import_from.from_names.at(0).name());
    ASSERT_EQ("b", import_from.from_names.at(1).name());

    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);

    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);

    auto import_as_name1 = import_as_name_vec.at(1);
    ASSERT_EQ("d", import_as_name1.name.name());
    ASSERT_FALSE(import_as_name1.opt_as_name);
}

TEST(ImportStmt, ImportFromRelativeMultipleAs) {
    constexpr auto input = R"(
from .a.b import c, d as c2
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(1, import_from.dot_num);
    ASSERT_EQ("a", import_from.from_names.at(0).name());
    ASSERT_EQ("b", import_from.from_names.at(1).name());

    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);

    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);

    auto import_as_name1 = import_as_name_vec.at(1);
    ASSERT_EQ("d", import_as_name1.name.name());
    ASSERT_TRUE(import_as_name1.opt_as_name);
    ASSERT_EQ("c2", import_as_name1.opt_as_name.value().name());
}

TEST(ImportStmt, ImportFromWildcard) {
    constexpr auto input = R"(
from .a.b import *
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    std::get<ImportFrom::Wildcard>(import_from.wildcard_or_import_as_names);
}

TEST(ImportStmt, ImportFromDot) {
    constexpr auto input = R"(
from . import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(1, import_from.dot_num);
    ASSERT_EQ(0, import_from.from_names.size());
    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);
    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);
}

TEST(ImportStmt, ImportFromDot2) {
    constexpr auto input = R"(
from .. import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(2, import_from.dot_num);
    ASSERT_EQ(0, import_from.from_names.size());
    auto import_as_name_vec =
        std::get<ImportAsNameVec>(import_from.wildcard_or_import_as_names);
    auto import_as_name0 = import_as_name_vec.at(0);
    ASSERT_EQ("c", import_as_name0.name.name());
    ASSERT_FALSE(import_as_name0.opt_as_name);
}

TEST(ImportStmt, ImportFromDot3) {
    constexpr auto input = R"(
from ... import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(3, import_from.dot_num);
}

TEST(ImportStmt, ImportFromDot5) {
    constexpr auto input = R"(
from ..... import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(5, import_from.dot_num);
}

TEST(ImportStmt, ImportFromDot6) {
    constexpr auto input = R"(
from ...... import c
)";
    auto result = parse_until_end(sg.import_stmt, input);
    ASSERT_TRUE(result) << result.error();

    auto import_from =
        std::get<ImportFrom>(result.success().import_or_import_from);

    ASSERT_EQ(6, import_from.dot_num);
}

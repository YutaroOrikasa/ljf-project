// #include "../common.hpp"

// #include "ljf-python/grammar/stmt.hpp"

// using namespace ljf::python::ast;

// TEST(ClassDef, Pass)
// {
//     constexpr auto input = R"(
// class A:
//     pass
// )";
//     auto result = parse_until_end(sg.import_stmt, input);
//     ASSERT_TRUE(result) << result.error();

//     auto import = std::get<DottedAsNameVec>(result.success().import_or_import_from).at(0);

//     ASSERT_EQ("a", import.names.at(0).name());

//     ASSERT_FALSE(import.opt_as_name);
// }

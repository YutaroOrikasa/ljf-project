#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(PassStmt, Pass)
{
    constexpr auto input = R"(
pass
)";
    auto result = parse_until_end(sg.pass_stmt, input);
    ASSERT_TRUE(result) << result.error();

}

#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(IfStmt, OneLine)
{
    auto result = parse_until_end(sg.if_stmt, "if True: a = 0");
    ASSERT_TRUE(result) << result.error();
}

TEST(IfStmt, IfElse)
{
    constexpr auto input = R"(
if x:
    a = 0
else:
    a = 1
)";
    auto result = parse_until_end(sg.if_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(ForStmt, ForStmt)
{
    constexpr auto input = R"(
for i in range(10):
    print(i)

)";
    auto result = parse_until_end(sg.for_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(ForStmt, Break)
{
    constexpr auto input = R"(
for i in range(10):
    break
)";
    auto result = parse_until_end(sg.for_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(ForStmt, Continue)
{
    constexpr auto input = R"(
for i in range(10):
    continue
)";
    auto result = parse_until_end(sg.for_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(ForStmt, ContinueInIfStmt)
{
    constexpr auto input = R"(
for i in range(10):
    if i % 2 == 1:
        continue
    print(i)
)";
    auto result = parse_until_end(sg.for_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

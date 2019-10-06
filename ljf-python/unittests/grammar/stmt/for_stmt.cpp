#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(ForStmt, ForStmt)
{
    constexpr auto input = R"(
for i in range(10):
    print(i)

)";
    auto result = parse_until_end(sg.if_stmt, input);
    ASSERT_TRUE(result) << result.error();
}

#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(ExprStmt, Assign)
{
    auto result = parse_until_end(sg.expr_stmt, "a = 0");
    ASSERT_TRUE(result);
}

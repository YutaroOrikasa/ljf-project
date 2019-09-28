#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(IfStmt, OneLine)
{
    auto result = parse_until_end(sg.if_stmt, "if True: a = 0");
    ASSERT_TRUE(result);
}

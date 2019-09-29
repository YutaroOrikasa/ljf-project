#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"


TEST(ExprStmt, Expr)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "a");
    ASSERT_TRUE(result) << result.error();

}

TEST(ExprStmt, Assign)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "a = 0");
    ASSERT_TRUE(result) << result.error();

}

TEST(ExprStmt, MultiAssign)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "a = b = c = 0");
    ASSERT_TRUE(result) << result.error();

}

TEST(ExprStmt, TargetListAssign)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "a, b, c = 1, 2, 3");
    ASSERT_TRUE(result) << result.error();

}

TEST(ExprStmt, ParenthTargetListAssign)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "(a, b, c) = (1, 2, 3)");
    ASSERT_TRUE(result) << result.error();

}

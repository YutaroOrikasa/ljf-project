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

TEST(ExprStmt, AssignBuiltinConstant)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "a = None");
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

TEST(ExprStmt, InvalidStmtAssignToKeyword)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "if = 0");
    ASSERT_FALSE(result);
}

TEST(ExprStmt, InvalidStmtAssignToBuiltinConstant)
{
    ASSERT_TRUE(sg.expr_stmt.has_parser());
    auto result = parse_until_end(sg.expr_stmt, "None = 0");
    // The parser accsept statements assigning to builtin constant,
    // but that's okey because CPython's grammer definitions for pgen parser generator
    // are so.
    // Such statements should be checked by a varidator or a compiler.
    ASSERT_TRUE(result) << result.error();
    // ASSERT_FALSE(result);
}

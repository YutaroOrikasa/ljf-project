#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(ClassDef, Pass)
{
    constexpr auto input = R"(
class A:
    pass
)";
    auto result = parse_until_end(sg.classdef, input);
    ASSERT_TRUE(result) << result.error();

    ClassStmt cls = result.success();
    ASSERT_EQ("A", cls.classname.name());
    ASSERT_EQ(0, cls.inheritance_list.size());
    std::get<PassStmt>(cls.body.stmt_list_.at(0).stmt_variant());
}

TEST(ClassDef, Inheritance)
{
    constexpr auto input = R"(
class A(B):
    pass
)";
    auto result = parse_until_end(sg.classdef, input);
    ASSERT_TRUE(result) << result.error();

    ClassStmt cls = result.success();
    ASSERT_EQ(1, cls.inheritance_list.size());
    // std::variant<Arg, KeywordArg, Comprehension> arg_var
    auto arg_var = cls.inheritance_list.at(0).arg_var;
    ASSERT_EQ(0, arg_var.index());

    auto arg = std::get<Argument::Arg>(cls.inheritance_list.at(0).arg_var);
    auto test = std::get<ConditionalExpr>(arg.expr.expr_variant());
    auto comparison = std::get<AtomExpr>(test.or_test_.expr_variant());

    auto base_class = std::get<IdentifierExpr>(comparison.atom_.expr_variant());
    ASSERT_EQ("B", base_class.name());
    std::get<PassStmt>(cls.body.stmt_list_.at(0).stmt_variant());
}

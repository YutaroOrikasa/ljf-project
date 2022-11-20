#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

TEST(FuncDef, FuncDef) {
    constexpr auto input = R"(
def f(a):
    a = a
    return a
)";
    auto result = parse_until_end(sg.funcdef, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(FuncDef, EmptyReturn) {
    constexpr auto input = R"(
def f(a):
    a = a
    return
)";
    auto result = parse_until_end(sg.funcdef, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(FuncDef, NoReturn) {
    constexpr auto input = R"(
def f(a):
    a = a
)";
    auto result = parse_until_end(sg.funcdef, input);
    ASSERT_TRUE(result) << result.error();
}

TEST(FuncParams, FuncParams) {
    constexpr auto input = "(a, b, c)";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    ASSERT_EQ(3, params.def_params_.size());
    EXPECT_EQ("a", params.def_params_[0].name.name());
    EXPECT_FALSE(params.def_params_[0].opt_default_value);
    EXPECT_FALSE(params.opt_starred_param_);
    EXPECT_FALSE(params.opt_double_starred_param_);
}

TEST(FuncParams, FuncParamsEndsWithComma) {
    constexpr auto input = "(a, b, c, )";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    ASSERT_EQ(3, params.def_params_.size());
    EXPECT_EQ("a", params.def_params_[0].name.name());
    EXPECT_FALSE(params.def_params_[0].opt_default_value);
    EXPECT_FALSE(params.opt_starred_param_);
    EXPECT_FALSE(params.opt_double_starred_param_);
}

TEST(FuncParams, NoParameters) {
    constexpr auto input = "()";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();

    const ast::FuncParams &params = result.success();
    ASSERT_EQ(0, params.def_params_.size());
}

TEST(FuncParams, HasDefaultValue) {
    constexpr auto input = "(a, b=0)";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    ASSERT_EQ(2, params.def_params_.size());
    EXPECT_EQ("b", params.def_params_[1].name.name());
    EXPECT_TRUE(params.def_params_[1].opt_default_value);
}

TEST(FuncParams, StarredParam) {
    constexpr auto input = "(*args)";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    EXPECT_TRUE(params.opt_starred_param_);
    EXPECT_FALSE(params.opt_double_starred_param_);
}

TEST(FuncParams, StarredAndDoubleStarred) {
    constexpr auto input = "(*args, **kwargs)";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    EXPECT_TRUE(params.opt_starred_param_);
    EXPECT_TRUE(params.opt_double_starred_param_);
}

TEST(FuncParams, DefParamStarredAndDoubleStarred) {
    constexpr auto input = "(a=0, *args, **kwargs)";
    auto result = parse_until_end(sg.parameters, input);
    ASSERT_TRUE(result) << result.error();
    const ast::FuncParams &params = result.success();

    ASSERT_EQ(1, params.def_params_.size());
    EXPECT_TRUE(params.def_params_[0].opt_default_value);
    EXPECT_TRUE(params.opt_starred_param_);
    EXPECT_TRUE(params.opt_double_starred_param_);
}

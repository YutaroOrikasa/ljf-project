
#include "gtest/gtest.h"

#include "ljf-python/grammar/expr.hpp"
#include "ljf-python/tokenizer.hpp"

using namespace ljf::python;
using namespace ljf::python::grammar;

using namespace ljf::python::parser;

static ExprGrammars<SStreamTokenStream> eg;

template <typename Parser, typename TokenStream>
static auto parse_until_eof(const Parser &p, TokenStream &stream) {
    return (p + separator(eof))(stream);
}

template <typename Parser>
static auto parse_until_end(const Parser &p, std::string input) {
    SStreamTokenStream ts{input};
    return parse_until_eof(p, ts);
}

TEST(TestExpr, Ident) {
    SStreamTokenStream ts{"a"};
    ExprGrammars<SStreamTokenStream> eg;
    auto result = eg.test(ts);
    ASSERT_TRUE(result);
}

TEST(TestListExpr, TwoItems) {
    SStreamTokenStream ts{"a, b"};
    ExprGrammars<SStreamTokenStream> eg;
    auto result = eg.testlist(ts);
    ASSERT_TRUE(result);
}

TEST(TestListExpr, BadInputTwoIdent) {
    auto result = parse_until_end(eg.testlist, "a b");
    ASSERT_FALSE(result);
}

TEST(TestNoCondExpr, BadInputCondExpr) {
    SStreamTokenStream ts{"x if a else y"};
    ExprGrammars<SStreamTokenStream> eg;
    auto result = parse_until_eof(eg.test_nocond, ts);
    ASSERT_FALSE(result);
}

// TEST(CondExpr, CondExpr)
// {
//     SStreamTokenStream ts{"x if a else y"};
//     ExprGrammars<SStreamTokenStream> eg;
//     auto result = eg.testlist(ts);
//     ASSERT_TRUE(result);
// }


#include "gtest/gtest.h"

#include "ljf-python/tokenizer.hpp"
#include "ljf-python/grammar/expr.hpp"

using namespace ljf::python;
using namespace ljf::python::grammar;

TEST(TestExpr, Ident)
{
    SStreamTokenStream ssts{"a"};
    ExprGrammars<SStreamTokenStream> eg;
    auto result = eg.test(ssts);
    ASSERT_TRUE(result);
}

#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(Stmt, All)
{
    constexpr auto input = R"(
import a
from .a import b

1 + 1

class A:
    def f(self):
        for i in range(10):
            if x == 1:
                continue
            elif x == 2:
                break
            else:
                pass
)";
    auto result = parse_until_end(sg.stmt, input);
    ASSERT_TRUE(result) << result.error();

}

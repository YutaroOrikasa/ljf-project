#include "../common.hpp"

#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python::ast;

TEST(FileInput, HasNewline)
{
    constexpr auto input = R"(
pass

pass


pass
)";
    auto result = parse_until_end(sg.file_input, input);
    ASSERT_TRUE(result) << result.error();
    ASSERT_EQ(3, result.success().stmt_list_.size());

}

TEST(FileInput, NotHasNewline)
{
    constexpr auto input = R"(pass
pass
pass
)";
    auto result = parse_until_end(sg.file_input, input);
    ASSERT_TRUE(result) << result.error();
    ASSERT_EQ(3, result.success().stmt_list_.size());

}

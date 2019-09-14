
#include <type_traits>
#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

using namespace ljf::python;
using namespace ljf::python::ast;

class Visitor
{
private:
    auto impl(const ListExpr &expr) const
    {
        std::cout << "ListExpr"
                  << "\n";
    }

public:
    bool operator()(const Token &token) const
    {
        if (token.is_eof())
        {
            return true;
        }
        std::cout << "Token: " << token.str() << "\n";
        return false;
    }
    template <typename T>
    bool operator()(const T &t) const
    {
        impl(t);
        return false;
    }
};

int main(int argc, const char **argv)
{

    // discard tokens other than NEWLINE
    // and then, discard NEWLINE itself
    constexpr auto discard_until_end_of_line = [](auto &&token_stream) -> void {
        while (!token_stream.peek().is_newline())
        {
            if (token_stream.peek().is_eof())
            {
                return;
            }
            std::cout << "discard `" << token_stream.peek().str() << "`\n";
            token_stream.read();
        }

        assert(token_stream.peek().is_newline());
        token_stream.read();
    };
    std::cout << "Read Parse Print Loop" << std::endl;
    using namespace ljf::python::parser;
    constexpr auto program = atom | eof;

    TokenStream<std::istream> ts{std::cin};

    // int dummy = result_content_t<decltype(program), decltype(ts)>();

    bool eof = false;
    while (!eof)
    {
        auto result = program(ts);
        if (result.failed())
        {
            std::cout << result.error() << "\n";
            discard_until_end_of_line(ts);
            continue;
        }
        eof = result.visit(Visitor());
    }
}

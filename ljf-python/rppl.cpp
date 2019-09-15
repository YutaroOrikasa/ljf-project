
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

    auto impl(const ParenthFormExpr &expr) const
    {
        std::cout << "ParenthFormExpr"
                  << "\n";
    }

    auto impl(const StringLiteralExpr &expr) const
    {
        std::cout << "StringLiteralExpr: "
                  << expr.token().str()
                  << "\n";
    }

    auto impl(const IntegerLiteralExpr &expr) const
    {
        std::cout << "IntegerLiteralExpr: "
                  << expr.token().str()
                  << "\n";
    }

    auto impl(const IdentifierExpr &expr) const
    {
        std::cout << "IdentifierExpr: "
                  << expr.token().str()
                  << "\n";
    }

    auto impl(const std::any &any) const
    {
        std::cout << "Any: type: " << any.type().name()
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

    template <typename... Ts>
    bool operator()(const std::variant<Ts...> &var) const
    {
        return std::visit(*this, var);
    }

    template <typename T>
    bool operator()(const T &t) const
    {
        std::cout << "visit " << typeid(T).name() << "\n";
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
    const auto program = eof | make_expr_parser();

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

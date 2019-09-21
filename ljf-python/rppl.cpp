
#include <type_traits>
#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "grammer.hpp"

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

    auto impl(const UnaryExpr &expr) const
    {
        std::cout << "UnaryExpr: "
                  <<"'" << expr.operator_.str() << "'"
                  << " ";
        (*this)(expr.operand_);
    }

    template <typename T>
    auto impl(const T &t) const
    {
        std::cout << "Unknown type: " << typeid(t).name()
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

    bool operator()(const Expr &expr) const
    {
        std::cout << "visit Expr\n";
        return expr.accept(*this);
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
    // const auto program = eof | newline | make_expr_parser();
    const auto program = make_python_grammer_parser();

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

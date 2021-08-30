
#include <type_traits>
#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "grammar.hpp"
#include "grammar/expr.hpp"
#include "grammar/stmt.hpp"

using namespace ljf::python;
using namespace ljf::python::ast;
using namespace ljf::python::parser;
using namespace ljf::python::grammar;

/// return: bool: is eof
class Visitor
{
private:

public:
    bool operator()(const Token &token) const
    {
        if (token.is_eof())
        {
            return true;
        }

        if (token.is_newline())
        {
            std::cout << "<NEWLINE>";
            return false;
        }
        std::cout << "Token: `" << token.str() << "`";
        return false;
    }



    template <typename T>
    bool operator()(const T &t) const
    {
        std::cout << "Accept (" << typeid(t).name() << ")";
        return false;
    }

};

/// return: bool: is eof
class ResultSuccessVisitor
{
public:
    bool operator()(const Expr &expr) const
    {
        return expr.accept(Visitor());
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
    // const auto program_ = make_python_grammer_parser();
    // (void)program_;
    // const auto expr = make_python_eval_input_parser();
    StmtGrammars<IStreamTokenStream> S;
    const auto expr = S.exprlist;
    const auto program = expr + separator(newline);
    IStreamTokenStream ts{std::cin};

    // int dummy = result_content_t<decltype(program), decltype(ts)>();

    bool eof = false;
    while (!eof)
    {
        auto result = program(ts);
        if (result.failed())
        {
            if (result.error().token().is_eof())
            {
                return 0;
            }

            std::cout << result.error() << "\n";
            discard_until_end_of_line(ts);
            continue;
        }
        eof = result.visit(ResultSuccessVisitor());
        std::cout << "\nEND";
        std::cout << "\n";
    }
}

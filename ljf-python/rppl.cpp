
#include <type_traits>
#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "grammar.hpp"
#include "grammar/expr.hpp"

using namespace ljf::python;
using namespace ljf::python::ast;
using namespace ljf::python::parser;
using namespace ljf::python::grammar;

template <typename List, typename Visitor>
auto &print_list(const List &list, Visitor &&elem_printer)
{
    bool is_first = true;
    for (auto &&elem : list)
    {
        if (is_first)
        {
            is_first = false;
        }
        else
        {
            std::cout << ", ";
        }
        elem_printer(elem);
    }

    return std::cout;
}

class Visitor
{
private:
    auto impl(const ListExpr &expr) const
    {
        std::cout << "[";
        print_list(expr.expr_list_, *this);
        std::cout << "]";
    }

    auto impl(const TupleExpr &expr) const
    {
        std::cout << "tuple(";
        print_list(expr.expr_list_, *this);
        std::cout << ")";
    }

    auto impl(const AtomExpr &expr) const
    {
        if (expr.has_await_)
        {
            std::cout << "await ";
        }
        visit(expr.atom_);
        if (!expr.trailers_.empty())
        {
            std::cout << "<trailers>";
        }
    
    }

    auto impl(const StringLiteralExpr &expr) const
    {
        std::cout << expr.token().str();
    }

    auto impl(const IntegerLiteralExpr &expr) const
    {
        std::cout << expr.token().str();
    }

    auto impl(const IdentifierExpr &expr) const
    {
        std::cout << expr.token().str();
    }

    auto impl(const ConditionalExpr &expr) const
    {
        (*this)(expr.or_test_);
        if (expr.if_else_)
        {
            std::cout << " if " << (*this)(expr.if_else_->if_);
            std::cout << " else " << (*this)(expr.if_else_->else_);
        }
    }

    auto impl(const BinaryExpr &expr) const
    {
        std::cout << "(";
        visit(expr.left_);
        std::cout << " " << expr.operator_.str()
                  << " ";
        visit(expr.right_);
        std::cout << ")";
    }

    auto impl(const UnaryExpr &expr) const
    {
        std::cout << "[UnaryExpr: "
                  << expr.operator_.str()
                  << " ";
        (*this)(expr.operand_);
        std::cout << "]";
    }

    auto impl(const EmptySExpr &) const
    {
        std::cout << "()";
    }

    template <typename T>
    auto impl(const T &t) const
    {
        std::cout << "(Unknown type: " << typeid(t).name() << ")";
    }

    auto impl(const SExprList &s_expr_list) const
    {
        std::cout << "(";
        bool is_first = true;
        for (auto &&s_expr : s_expr_list)
        {
            if (is_first)
            {
                is_first = false;
            }
            else
            {
                std::cout << ", ";
            }
            (*this)(s_expr);
        }
        std::cout << ")";
    }

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

    template <typename... Ts>
    bool operator()(const std::variant<Ts...> &var) const
    {
        return std::visit(*this, var);
    }

    bool operator()(const Expr &expr) const
    {
        return expr.accept(*this);
    }

    bool operator()(const SExpr &expr) const
    {
        return expr.accept(*this);
    }

    template <typename T>
    bool operator()(const T &t) const
    {
        impl(t);
        return false;
    }

    template <typename T>
    bool visit(const T &t) const
    {
        return (*this)(t);
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
    ExprGrammars<IStreamTokenStream> E;
    const auto expr = E.exprlist;
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
        eof = result.visit(Visitor());
        std::cout << "\nEND";
        std::cout << "\n";
    }
}

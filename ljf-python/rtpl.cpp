
#include <iostream>

#include "tokenizer.hpp"

using namespace ljf::python;

template <typename Tokenizer>
auto /*std::vector*/ tokenize_line(Tokenizer &tokenizer, bool &eof)
{
    std::vector<Phase1Token> tokens;
    for (;;)
    {
        auto token = tokenizer.read();
        tokens.push_back(token);

        if (token.is_newline())
        {
            break;
        }

        if (token.is_eof())
        {
            eof = true;
            break;
        }
    }
    return tokens;
}

std::string token_to_str(const Token &t)
{
    switch (t.category())
    {
    case token_category::EOF_TOKEN:
        return "<EOF>";
    case token_category::INDENT:
        return "<INDENT>";
    case token_category::DEDENT:
        return "<DEDENT>";
    case token_category::NEWLINE:
        return "<NEWLINE>";
    case token_category::INVALID:
        return "<INVALID TOKEN: `" + t.str() + "`, error msg: " + t.error_message() + ">";
    case token_category::ANY_OTHER:
        return "<UNKNOWN TOKEN TYPE>`" + t.str() + "`";
    default:
        return "`" + t.str() + "`";
    }
}

template <typename Tokenizer>
void tokenize_line_and_print_it(Tokenizer &tokenizer, bool &eof, bool verbose)
{
    // tokenizer shows prompt on read()
    auto tokens = tokenize_line(tokenizer, eof);
    std::cout << "std::cout: [";
    bool start = true;
    for (auto &&token : tokens)
    {
        if (!start)
        {
            std::cout << ", ";
        }
        else
        {
            start = false;
        }

        if (verbose)
        {
            std::cout << token.source_location() << ":";
        }

        std::cout << token_to_str(token);
    }
    std::cout << "]" << std::endl;
}

int main(int argc, const char **argv)
{
    bool verbose = false;
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() >= 2 && args.at(1) == "-v")
    {
        verbose = true;
    }

    std::cout << "Read Tokenize Print Loop" << std::endl;

    ljf::python::TokenStream<std::istream> tokenizer{std::cin};
    // ljf::python::TokenStream<std::fstream> tokenizer{"pycode/tarai.py"};

    bool eof = false;
    while (!eof)
    {
        tokenize_line_and_print_it(tokenizer, eof, verbose);
    }
}

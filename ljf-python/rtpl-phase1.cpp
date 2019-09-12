
#include <iostream>

#include "tokenizer/phase1.hpp"

using namespace ljf::python;

template <typename Tokenizer>
auto /*std::vector*/ tokenize_line(Tokenizer &tokenizer, bool &eof)
{
    std::vector<Token> tokens;
    for (;;)
    {
        auto token = tokenizer.read();

        tokens.push_back(token);

        if (token == "\n")
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

template <typename Tokenizer>
void tokenize_line_and_print_it(Tokenizer &tokenizer, bool &eof, bool verbose)
{
    // tokenizer shows prompt on read()
    auto tokens = tokenize_line(tokenizer, eof);

    std::cout << "std::cout: [";
    for (auto &&token : tokens)
    {
        if (verbose)
        {
            std::cout << token.source_location() << ":";
        }

        std::cout << "`" << token.str() << "`"
                  << ", ";
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

    ljf::python::Phase1TokenStream<std::istream> tokenizer{std::cin};
    // ljf::python::Phase1TokenStream<std::fstream> tokenizer{"pycode/tarai.py"};

    bool eof = false;
    while (!eof)
    {
        tokenize_line_and_print_it(tokenizer, eof, verbose);
    }
}

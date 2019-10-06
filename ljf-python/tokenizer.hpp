#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <type_traits>
#include <istream>
#include <fstream>
#include <sstream>

#include "tokenizer/phase1.hpp"
#include "Token.hpp"

namespace ljf::python::detail::tokenizer::phase2
{

class IndentNester
{
private:
    std::string indent_;

public:
    IndentNester(const std::string &indent) : indent_(indent) {}

    size_t indent_width() const noexcept
    {
        return indent_.size();
    }
};

class BracketNester
{
};

struct Nester
{
private:
    using variant_type = std::variant<IndentNester, BracketNester>;

public:
    variant_type variant;

    template <typename T,
              std::enable_if_t<
                  std::is_constructible_v<
                      variant_type, T>> * = nullptr>
    /*implicit*/ Nester(T &&t) : variant(std::forward<T>(t))
    {
    }

    bool is_indent_nester() const noexcept
    {
        return std::holds_alternative<IndentNester>(variant);
    }

    const IndentNester &get_indent_nester() const noexcept
    {
        return std::get<IndentNester>(variant);
    }

    bool is_bracket_nester() const noexcept
    {
        return std::holds_alternative<BracketNester>(variant);
    }
};

} // namespace ljf::python::detail::tokenizer::phase2

namespace ljf::python
{

template <typename IStream>
class TokenStream
{
private:
    Phase1TokenStream<IStream> stream_;
    // std::optional<Token> last_token_;
    std::queue<Token> token_buffer_;
    std::size_t current_position_ = 0;
    bool is_prev_token_newline_ = false;

    using Nester = detail::tokenizer::phase2::Nester;
    using IndentNester = detail::tokenizer::phase2::IndentNester;
    // put empty IndentNester to ensure nester_stack_.size() >= 1
    std::vector<Nester> nester_stack_{detail::tokenizer::phase2::IndentNester("")};

public:
    template <typename S>
    TokenStream(S &&stream) : stream_(std::forward<S>(stream)) {}

    /// returns a Token and advances Phase1TokenStream's current position.
    /// tokenizing is executed one line at a time.
    Token read()
    {
        fill_token_buffer();

        ++current_position_;
        return dequeue();
    }

    /// returns a Token.
    /// tokenizing is executed one line at a time.
    /// This function is same as read() excapt not advancing Phase1TokenStream's current position.
    Token peek()
    {
        fill_token_buffer();

        return token_buffer_.front();
    }

    std::size_t current_position() const noexcept
    {
        return current_position_;
    }

    template <typename Str>
    void prompt(Str &&str)
    {

        stream_.prompt(std::forward<Str>(str));
    }

private:
    void enqueue(Token &&token)
    {
        is_prev_token_newline_ = token.is_newline();
        token_buffer_.push(std::move(token));
    }

    Token dequeue()
    {
        assert(!token_buffer_.empty());
        auto token = std::move(token_buffer_.front());
        token_buffer_.pop();
        return token;
    }

    bool is_prev_token_newline() const noexcept
    {
        return is_prev_token_newline_;
    }

    void fill_token_buffer()
    {

        using namespace detail::tokenizer::phase2;
        while (token_buffer_.empty())
        {
            auto token = stream_.read();
            if (token.category() == token_category::WHITESPACE_AT_BIGGINING_OF_LINE ||
                token.is_eof())
            {
                proccess_indentation(token);
                if (token.is_eof())
                {
                    enqueue(std::move(token));
                    return;
                }

                continue;
            }
            if (token.category() == token_category::OPENING_BRACKET)
            {
                nester_stack_.push_back(BracketNester());
            }
            if (token.category() == token_category::CLOSING_BRACKET)
            {
                proccess_closing_bracket();
            }
            if (token.category() == token_category::NEWLINE)
            {
                if (nester_stack_.back().is_bracket_nester())
                {
                    // skip newline in bracket
                    prompt("(in bracket)> ");
                    continue;
                }
                if (is_prev_token_newline())
                {
                    push_dedent_from_newline(std::move(token));
                }
            }
            enqueue(std::move(token));
        }
    }

    void proccess_closing_bracket()
    {
        if (nester_stack_.back().is_bracket_nester())
        {
            nester_stack_.pop_back();
        }
    }

    void proccess_indentation(const Token &token)
    {

        using namespace detail::tokenizer::phase2;

        std::cout << "WHITESPACE_AT_BIGGINING_OF_LINE\n";
        auto back = nester_stack_.back();
        if (!back.is_indent_nester())
        {
            return;
        }

        auto nester = IndentNester(token.str());
        auto top_nester = back.get_indent_nester();
        if (top_nester.indent_width() < nester.indent_width())
        {
            // indent level increased
            nester_stack_.push_back(nester);
            enqueue(Token::create_indent_token(token));
            return;
        }
        else if (top_nester.indent_width() == nester.indent_width())
        {
            std::cout << "indent level not changed\n";
            return;
        }
        else
        {
            assert(top_nester.indent_width() > nester.indent_width());
            // indent level decreased
            proccess_dedent(nester, token);
        }
    }

    void proccess_dedent(const IndentNester &nester, const Token &token)
    {
        while (true)
        {
            nester_stack_.pop_back();
            auto back = nester_stack_.back();
            assert(back.is_indent_nester());
            auto back_nester = back.get_indent_nester();
            if (back_nester.indent_width() >= nester.indent_width())
            {
                enqueue(Token::create_dedent_token(token));
                if (back_nester.indent_width() == nester.indent_width())
                {
                    return;
                }
            }
            else
            {
                enqueue(Token::create_invalid_token(token, "invalid dedent"));
                return;
            }
        }
    }

    void push_dedent_from_newline(Token &&token)
    {
        auto back = nester_stack_.back();
        assert(back.is_indent_nester());
        auto back_nester = back.get_indent_nester();
        if (back_nester.indent_width() == 0)
        {
            return;
        }

        nester_stack_.pop_back();
        enqueue(Token::create_dedent_token(token));
    }
};

using IStreamTokenStream = TokenStream<std::istream>;
using FStreamTokenStream = TokenStream<std::fstream>;
using SStreamTokenStream = TokenStream<std::stringstream>;

} // namespace ljf::python

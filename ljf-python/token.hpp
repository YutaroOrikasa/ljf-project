#pragma once

#include <string>

#include "SourceLocation.hpp"

namespace ljf::python
{

enum class token_category
{
    EOF_TOKEN,
    WHITESPACE_AT_BIGGINING_OF_LINE,
    INDENT,
    DEDENT,
    NEWLINE,
    OPENING_BRACKET,
    CLOSING_BRACKET,
    INVALID, // representing tokenizeing error
    ANY_OTHER
};

class Phase1Token
{
private:
    std::string token_;
    SourceLocation loc_;

    token_category token_category_;

    std::string error_message_;

    Phase1Token(const std::string &token, const SourceLocation &loc, token_category ty,
                const std::string &error_msg)
        : token_(token),
          loc_(loc),
          token_category_(ty),
          error_message_(error_msg) {}

public:
    Phase1Token(const std::string &token, const SourceLocation &loc, token_category ty)
        : token_(token),
          loc_(loc),
          token_category_(ty) {}

    static Phase1Token create_eof_token(const SourceLocation &loc)
    {
        return Phase1Token("", loc, token_category::EOF_TOKEN);
    }

    static Phase1Token create_indent_token(const Phase1Token &orig_token)
    {
        return Phase1Token(orig_token.str(), orig_token.source_location(), token_category::INDENT);
    }

    static Phase1Token create_dedent_token(const Phase1Token &orig_token)
    {
        return Phase1Token(orig_token.str(), orig_token.source_location(), token_category::DEDENT);
    }

    static Phase1Token create_invalid_token(const Phase1Token &orig_token, const std::string &error_msg)
    {
        return Phase1Token(orig_token.str(), orig_token.source_location(), token_category::INVALID, error_msg);
    }

    std::string str() const
    {
        return token_;
    }

    const std::string &error_message() const noexcept
    {
        return error_message_;
    }

    bool is_eof() const noexcept
    {
        return token_category_ == token_category::EOF_TOKEN;
    }

    bool is_indent() const noexcept
    {
        return token_category_ == token_category::INDENT;
    }

    bool is_dedent() const noexcept
    {
        return token_category_ == token_category::DEDENT;
    }

    bool is_newline() const noexcept
    {
        return token_category_ == token_category::NEWLINE;
    }

    bool is_invalid() const noexcept
    {
        return token_category_ == token_category::INVALID;
    }

    token_category category() const noexcept
    {
        return token_category_;
    }

    const SourceLocation &source_location() const noexcept
    {
        return loc_;
    }

    bool operator==(const Phase1Token &) = delete;

    template <typename Str>
    bool operator==(const Str &str) const
    {
        if (is_invalid())
        {
            return false;
        }

        return token_ == str;
    }

    template <typename Str>
    friend bool operator==(const Str &str, const Phase1Token &token)
    {
        return token == str;
    }

    template <typename T, typename U>
    friend bool operator!=(const T &t, const U &u)
    {
        return !(t == u);
    }
};
} // namespace ljf::python

#pragma once

#include <string>
#include <variant>

#include <cassert>

#include "SourceLocation.hpp"
#include "literals.hpp"

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
    ANY_OTHER,
    //
    START_HAVING_CONCRETE_DATA_, // This is for internal check, not public.
    STRING_LITERAL = START_HAVING_CONCRETE_DATA_,
    INVALID, // representing tokenizeing error
};

class Token
{
private:
    std::string token_;
    SourceLocation loc_;

    token_category token_category_;

    using error_msg_string = std::string;

    std::variant<std::monostate,
                 literals::StringLiteral,
                 error_msg_string>
        concrete_data_variant_;

    template <typename T>
    Token(const std::string &token, const SourceLocation &loc, token_category ty,
          T &&concrete_data)
        : token_(token),
          loc_(loc),
          token_category_(ty),
          concrete_data_variant_(std::forward<T>(concrete_data)) {}

    Token(const std::string &token, const SourceLocation &loc, token_category ty)
        : token_(token),
          loc_(loc),
          token_category_(ty)
    {
        assert(ty != token_category::INVALID);
    }

public:
    template <token_category C>
    static Token create_token(const std::string &str, const SourceLocation &loc)
    {
        static_assert(C < token_category::START_HAVING_CONCRETE_DATA_,
                      "please use create_XXX_literal_token() or create_invalid_token() instead");
        return Token(str, loc, C);
    }

    static Token create_eof_token(const SourceLocation &loc)
    {
        return Token("", loc, token_category::EOF_TOKEN);
    }

    static Token create_indent_token(const Token &orig_token)
    {
        return Token(orig_token.str(), orig_token.source_location(), token_category::INDENT);
    }

    static Token create_dedent_token(const Token &orig_token)
    {
        return Token(orig_token.str(), orig_token.source_location(), token_category::DEDENT);
    }

    static Token create_invalid_token(const Token &orig_token, const std::string &error_msg)
    {
        return Token(orig_token.str(), orig_token.source_location(), token_category::INVALID, error_msg);
    }

    static Token create_invalid_token(const std::string &str, const SourceLocation &loc, const std::string &error_msg)
    {
        return Token(str, loc, token_category::INVALID, error_msg);
    }

    static Token create_string_literal_token(const std::string &entire,
                                             const std::string &prefix,
                                             const std::string &contents,
                                             const SourceLocation &loc)
    {
        return Token(entire,
                     loc,
                     token_category::STRING_LITERAL,
                     literals::StringLiteral(prefix, contents));
    }

    const std::string &str() const
    {
        return token_;
    }

    const std::string &error_message() const noexcept
    {
        assert(is_invalid());
        assert(std::holds_alternative<error_msg_string>(concrete_data_variant_));
        return std::get<error_msg_string>(concrete_data_variant_);
    }

    const literals::StringLiteral &get_string_literal() const
    {
        assert(is_string_literal());
        return std::get<literals::StringLiteral>(concrete_data_variant_);
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

    bool is_string_literal() const noexcept
    {
        return token_category_ == token_category::STRING_LITERAL;
    }

    token_category category() const noexcept
    {
        return token_category_;
    }

    const SourceLocation &source_location() const noexcept
    {
        return loc_;
    }

    bool operator==(const Token &) = delete;

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
    friend bool operator==(const Str &str, const Token &token)
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

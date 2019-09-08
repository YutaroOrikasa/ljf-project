#pragma once

#include <string>
// #include <string_view>
#include <utility>
#include <vector>
#include <queue>
#include <regex>
#include <iostream>
#include <fstream>

#include <cstddef>
#include <cassert>

namespace ljf::python::detail
{
class StdIStreamWrapper
{
    std::istream &istream_;
    std::string prompt_;

public:
    /*implicit*/ StdIStreamWrapper(std::istream &is) : istream_(is) {}

    std::string getline()
    {
        std::string s;
        std::getline(istream_, s);
        prompt(prompt_);
        return s;
    }

    bool eof()
    {
        return istream_.eof();
    }

    // return: old prompt
    template <typename Str>
    std::string set_prompt(Str &&str)
    {
        auto old = prompt_;
        prompt_ = std::forward<Str>(str);
        return old;
    }

    template <typename Str>
    void prompt(Str &&str)
    {
        auto out_p = istream_.tie();
        if (!out_p)
        {
            return;
        }

        auto &out = *out_p;
        out << std::forward<Str>(str);
    }
};

class StdFStreamWrapper
{
    std::fstream fstream_;

public:
    /*implicit*/ StdFStreamWrapper(std::fstream &&fs) : fstream_(std::move(fs)) {}

    std::string getline()
    {
        std::string s;
        std::getline(fstream_, s);
        return s;
    }

    bool eof()
    {
        return fstream_.eof();
    }

    template <typename Str>
    void prompt(Str &&)
    {
        // do nothing
    }
};
} // namespace ljf::python::detail

namespace ljf::python
{

struct zero_based_index_t
{
};
inline constexpr zero_based_index_t zero_based_index;

struct one_based_index_t
{
};
inline constexpr one_based_index_t one_based_index;

class SourceLocation
{
    std::string source_file_name_;
    // row_ and col_ start at 1
    size_t row_ = -1; // set -1 as uninitialized value
    size_t col_ = -1; // set -1 as uninitialized value

public:
    explicit SourceLocation(one_based_index_t,
                            const std::string &source_file_name,
                            size_t row,
                            size_t col)
        : source_file_name_(source_file_name),
          row_(row),
          col_(col) {}

    explicit SourceLocation(zero_based_index_t,
                            const std::string &source_file_name,
                            size_t row,
                            size_t col)
        : source_file_name_(source_file_name),
          row_(row + 1),
          col_(col + 1) {}

    const std::string &source_file_name() const noexcept
    {
        return source_file_name_;
    }

    /// returned value is one-based index
    size_t row() const noexcept
    {
        return row_;
    }

    /// returned value is one-based index
    size_t column() const noexcept
    {
        return col_;
    }

    template <typename Out>
    friend Out &operator<<(Out &out, const SourceLocation &loc)
    {
        return out << loc.source_file_name() << ":" << loc.row() << ":" << loc.column();
    }
};

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

template <typename IStream>
class Phase1TokenStream
{
private:
    IStream stream_;
    std::queue<Phase1Token> token_buffer_;
    std::regex re{R"((^[ \t]*\n)|(^[ \t]*)|(def|class)|[\w]+|)"
                  R"("""(?:.|\n)*?"""|'''(?:.|\n)*?'''|((?:"""|''').*\n))" // triple quote kinds
                  R"(|([(\[{]))"                                           // opening brackets
                  R"(|([)\]}]))"                                           // closing brackets
                  R"(|(\\\n))"
                  R"(|(#.*\n)|"[^"\n]*"|(\n)|\S)"};
    struct sub_match_index
    {
        enum sub_match_index_enum : size_t
        {
            ZERO = 0, // this is dummy, exists so that next enum starts with 1
            EMPTY_LINE,
            WHITESPACE_AT_BIGGINING_OF_LINE,
            PYTHON_KEYWORD,
            CONTINUOUS_TRIPLE_QUOTE,
            OPENING_BRACKET,
            CLOSING_BRACKET,
            EXPLICIT_LINE_CONTINUATION,
            COMMENT,
            NEWLINE
        };
    };

    const std::string source_file_name_ = "<input>";
    // row and col of current input position.
    // index is start from 0.
    size_t row_ = 0;
    size_t col_ = 0;

public:
    template <typename S>
    Phase1TokenStream(S &&stream) : stream_(std::forward<S>(stream)) {}

    /// returns a Phase1Token and advances Phase1TokenStream's current position.
    /// tokenizing is executed one line at a time.
    Phase1Token read()
    {
        fill_token_buffer();
        if (token_buffer_.empty())
        {
            return create_eof_token();
        }

        return dequeue();
    }

    /// returns a Phase1Token.
    /// tokenizing is executed one line at a time.
    /// This function is same as read() excapt not advancing Phase1TokenStream's current position.
    Phase1Token peek()
    {
        fill_token_buffer();
        if (token_buffer_.empty())
        {
            return create_eof_token();
        }

        return token_buffer_.front();
    }

    template <typename Str>
    void prompt(Str &&str)
    {

        stream_.prompt(std::forward<Str>(str));
    }

    template <typename Str>
    void set_prompt(Str &&str)
    {

        stream_.set_prompt(std::forward<Str>(str));
    }

private:
    void enqueue(Phase1Token &&token)
    {
        token_buffer_.push(std::move(token));
    }

    Phase1Token dequeue()
    {
        assert(!token_buffer_.empty());
        auto token = std::move(token_buffer_.front());
        token_buffer_.pop();
        return token;
    }

    void enqueue_all(std::vector<Phase1Token> &&tokens)
    {
        for (auto &&token : tokens)
        {
            enqueue(std::move(token));
        }
        tokens.clear();
    }

    /// tokenize a line and enqueue result when token_buffer_ is empty.
    /// if the line ends in the middle of continuous row string line,
    /// this function will read multiple lines and tokenize.
    void fill_token_buffer()
    {

        if (!token_buffer_.empty())
        {
            return;
        }
        // `lines` is normally single line,
        // but it'll be multiple lines if the first line ends with continuous triple quote
        // (eg. """chars\n)
        std::string lines;

        while (true)
        {
            if (lines.empty())
            {
                prompt(">>> ");
            }
            else
            {
                prompt("... ");
            }

            {
                std::string s = stream_.getline();
                if (s.empty() && stream_.eof())
                {
                    enqueue(create_eof_token());
                    return;
                }

                s.append("\n");
                lines.append(s);
            }

            // it = begin
            auto it = std::sregex_iterator(lines.cbegin(), lines.cend(), re);
            auto end = std::sregex_iterator();
            bool has_continuous_line = false;
            std::vector<Phase1Token> tokens;
            for (; it != end; ++it)
            {
                auto &match_result = *it;
                if (match_result[sub_match_index::EMPTY_LINE].matched)
                {
                    // ignore it
                    continue;
                }
                if (match_result[sub_match_index::WHITESPACE_AT_BIGGINING_OF_LINE].matched)
                {
                    enqueue(Phase1Token(match_result.str(), get_current_source_location(), token_category::WHITESPACE_AT_BIGGINING_OF_LINE));
                    continue;
                }
                if (match_result[sub_match_index::COMMENT].matched)
                {
                    tokens.push_back(Phase1Token(match_result.str(), get_current_source_location(), token_category::NEWLINE));
                    continue;
                }
                if (match_result[sub_match_index::NEWLINE].matched)
                {
                    tokens.push_back(Phase1Token(match_result.str(), get_current_source_location(), token_category::NEWLINE));
                    continue;
                }
                if (match_result[sub_match_index::OPENING_BRACKET].matched)
                {
                    tokens.push_back(Phase1Token(match_result.str(), get_current_source_location(), token_category::OPENING_BRACKET));
                    continue;
                }
                if (match_result[sub_match_index::CLOSING_BRACKET].matched)
                {
                    tokens.push_back(Phase1Token(match_result.str(), get_current_source_location(), token_category::CLOSING_BRACKET));
                    continue;
                }
                if (match_result[sub_match_index::EXPLICIT_LINE_CONTINUATION].matched)
                {
                    // ignore it, don't push NEWLINE token
                    prompt("(line continuation) ");
                    break;
                }
                if (match_result[sub_match_index::CONTINUOUS_TRIPLE_QUOTE].matched)
                {
                    has_continuous_line = true;
                    continue;
                }
                tokens.push_back(Phase1Token(match_result.str(), get_current_source_location(), token_category::ANY_OTHER));
            }
            if (has_continuous_line)
            {
                // continue while(true) until raw string closing quote (corespondind """ or ''') found
                continue;
            }
            if (tokens.empty())
            {
                // continue while(true) until get non empty line
                lines.clear();
                continue;
            }

            assert(!tokens.empty());
            enqueue_all(std::move(tokens));
            // break while(true) loop
            return;
        }
    }

    SourceLocation get_current_source_location()
    {
        return SourceLocation(zero_based_index, source_file_name_, row_, col_);
    }

    Phase1Token create_eof_token()
    {
        return Phase1Token::create_eof_token(SourceLocation(zero_based_index, source_file_name_, 0, 0));
    }
};

template <>
class Phase1TokenStream<std::istream> : public Phase1TokenStream<detail::StdIStreamWrapper>
{
    using base_type = Phase1TokenStream<detail::StdIStreamWrapper>;

public:
    using base_type::base_type;
};

template <>
class Phase1TokenStream<std::fstream> : public Phase1TokenStream<detail::StdFStreamWrapper>
{
    using base_type = Phase1TokenStream<detail::StdFStreamWrapper>;

public:
    template <
        typename... Args,
        std::enable_if_t<
            std::is_constructible_v<
                std::fstream, Args...>> * = nullptr>
    explicit Phase1TokenStream(Args &&... args)
        : base_type(std::fstream(std::forward<Args>(args)...))
    {
    }
};
} // namespace ljf::python

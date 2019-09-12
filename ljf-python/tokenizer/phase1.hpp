#pragma once

#include <string>
// #include <string_view>
#include <utility>
#include <vector>
#include <queue>
#include <regex>

#include <cstddef>
#include <cassert>

#include "../SourceLocation.hpp"
#include "../Token.hpp"
#include "../std_stream_wrappers.hpp"

namespace ljf::python
{

template <typename IStream>
class Phase1TokenStream
{
private:
    IStream stream_;
    std::queue<Token> token_buffer_;
    std::regex re{
        R"((^[ \t]*\n))"                                                                      // EMPTY_LINE
        R"(|(^[ \t]*))"                                                                       // WHITESPACE_AT_BIGGINING_OF_LINE
        R"(|(def|class))"                                                                     // PYTHON_KEYWORD
        R"(|(b|B)?("""(?:.|\n)*?"""|'''(?:.|\n)*?'''|((?:"""|''').*\n)|"[^"\n]*"|'[^'\n]*'))" // BYTES_LITERAL_PREFIX, QUOTED, CONTINUOUS_TRIPLE_QUOTE
        R"(|([(\[{]))"                                                                        // OPENING_BRACKET
        R"(|([)\]}]))"                                                                        // CLOSING_BRACKET
        R"(|(\\\n))"                                                                          // EXPLICIT_LINE_CONTINUATION
        R"(|(#.*\n))"                                                                         // COMMENT
        R"(|(\n))"                                                                            // NEWLINE
        R"(|([^[:space:]\n\\]+))"                                                             // ANY_OTHER
    };
    struct sub_match_index
    {
        enum sub_match_index_enum : size_t
        {
            ZERO = 0, // this is dummy, exists so that next enum starts with 1
            EMPTY_LINE,
            WHITESPACE_AT_BIGGINING_OF_LINE,
            PYTHON_KEYWORD,
            //
            BYTES_LITERAL_PREFIX,
            QUOTED,
            CONTINUOUS_TRIPLE_QUOTE,
            //
            OPENING_BRACKET,
            CLOSING_BRACKET,
            EXPLICIT_LINE_CONTINUATION,
            COMMENT,
            NEWLINE,
            ANY_OTHER
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

    /// returns a Token and advances Phase1TokenStream's current position.
    /// tokenizing is executed one line at a time.
    Token read()
    {
        fill_token_buffer();
        if (token_buffer_.empty())
        {
            return create_eof_token();
        }

        return dequeue();
    }

    /// returns a Token.
    /// tokenizing is executed one line at a time.
    /// This function is same as read() excapt not advancing Phase1TokenStream's current position.
    Token peek()
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
    void enqueue(Token &&token)
    {
        token_buffer_.push(std::move(token));
    }

    Token dequeue()
    {
        assert(!token_buffer_.empty());
        auto token = std::move(token_buffer_.front());
        token_buffer_.pop();
        return token;
    }

    void enqueue_all(std::vector<Token> &&tokens)
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
            std::vector<Token> tokens;
            for (; it != end; ++it)
            {
                auto &match_result = *it;
                if (match_result[sub_match_index::EMPTY_LINE].matched)
                {
                    // ignore it
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
                token_category cat = match_result_to_token_category(match_result);
                tokens.push_back(Token(match_result.str(), get_current_source_location(), cat));
                std::cout << "sub_match_index=" << get_first_sub_match_index(match_result) << ", match_result.str()=" << match_result.str() << ", cat=" << int(cat) << "\n";
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

    static token_category match_result_to_token_category(const std::smatch &match_result)
    {
        auto index = get_first_sub_match_index(match_result);
        assert(index != 0);
        switch (index)
        {
        case sub_match_index::WHITESPACE_AT_BIGGINING_OF_LINE:
            return token_category::WHITESPACE_AT_BIGGINING_OF_LINE;

        case sub_match_index::COMMENT:
        case sub_match_index::NEWLINE:
            return token_category::NEWLINE;

        case sub_match_index::OPENING_BRACKET:
            return token_category::OPENING_BRACKET;

        case sub_match_index::CLOSING_BRACKET:
            return token_category::CLOSING_BRACKET;

        default:
            return token_category::INVALID;
        }
    }

    static size_t get_first_sub_match_index(const std::smatch &match_result)
    {
        // not i = 0, match_result[0] is not a sub match.
        for (size_t i = 1; i < match_result.size(); ++i)
        {
            if (match_result[i].matched)
            {
                return i;
            }
        }
        assert(false && "never come here, invalid match_result given");
    }

    SourceLocation get_current_source_location()
    {
        return SourceLocation(zero_based_index, source_file_name_, row_, col_);
    }

    Token create_eof_token()
    {
        return Token::create_eof_token(SourceLocation(zero_based_index, source_file_name_, 0, 0));
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

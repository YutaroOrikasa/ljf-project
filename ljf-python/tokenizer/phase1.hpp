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
        R"((^\n))"                                                      // EMPTY_LINE
        R"(|(^[ \t]*\n))"                                               // WHITESPACE_LINE
        R"(|(^[ \t]*))"                                                 // WHITESPACE_AT_BIGGINING_OF_LINE
        R"(|(def|class))"                                               // PYTHON_KEYWORD
        ""                                                              //
        R"(|((b|B)?(?:)"                                                // STRING_LITERAL, BYTES_LITERAL_PREFIX, (start (?:))
        R"_("""((?:.|\n)*?)""")_"                                       // TRIPLE_DOUBLE_QUOTED_CONTENTS
        R"_(|'''((?:.|\n)*?)''')_"                                      // TRIPLE_SINGLE_QUOTED_CONTENTS
        R"_(|((?:"""|''').*\n))_"                                       // CONTINUOUS_TRIPLE_QUOTE
        R"_(|"([^"\n]*)"|'([^'\n]*)')_"                                 // DOUBLE_QUOTED_CONTENTS, SINGLE_QUOTED_CONTENTS
        "))"                                                            // (end (?:)), (end STRING_LITERAL)
        ""                                                              //
        "|(0[bB](?:_?[0-1])+)"                                          // BIN_INTEGER_LITERAL
        "|(0[oO](?:_?[0-7])+)"                                          // OCT_INTEGER_LITERAL
        "|(0[xX](?:_?[0-9a-fA-F])+)"                                    // HEX_INTEGER_LITERAL
        "|([1-9](?:_?[0-9])*|0+(?:_?0)*)"                               // DEC_INTEGER_LITERAL
        ""                                                              //
        R"(|([(\[{]))"                                                  // OPENING_BRACKET
        R"(|([)\]}]))"                                                  // CLOSING_BRACKET
        R"(|(\\\n))"                                                    // EXPLICIT_LINE_CONTINUATION
        R"(|(#.*\n))"                                                   // COMMENT
        R"(|(\n))"                                                      // NEWLINE
        R"(|([+\-*/%&|~^@]=|(?:\*\*|//|<<|>>)=)"                        // DELIMITER
        R"_(|<=|>=|==|!=|\*\*|//|<<|>>|->|\.\.\.|[+\-*/%&|~^@,=:.;]))_" // DELIMITER (cont.)
        R"(|([^[:space:]+\-*/%&|~^@<>=!,:.;(){}[\]$?`\n\\]+))"          // IDENTIFIER
        R"(|(\S+))"                                                     // INVALID_TOKEN
    };
    struct sub_match_index
    {
        enum sub_match_index_enum : size_t
        {
            ZERO = 0, // this is dummy, exists so that next enum starts with 1
            EMPTY_LINE,
            WHITESPACE_LINE,
            WHITESPACE_AT_BIGGINING_OF_LINE,
            PYTHON_KEYWORD,
            //
            STRING_LITERAL,
            BYTES_LITERAL_PREFIX,
            TRIPLE_DOUBLE_QUOTED_CONTENTS,
            TRIPLE_SINGLE_QUOTED_CONTENTS,
            CONTINUOUS_TRIPLE_QUOTE,
            DOUBLE_QUOTED_CONTENTS,
            SINGLE_QUOTED_CONTENTS,
            //
            BIN_INTEGER_LITERAL,
            OCT_INTEGER_LITERAL,
            HEX_INTEGER_LITERAL,
            DEC_INTEGER_LITERAL,
            //
            OPENING_BRACKET,
            CLOSING_BRACKET,
            EXPLICIT_LINE_CONTINUATION,
            COMMENT,
            NEWLINE,
            DELIMITER,
            IDENTIFIER,
            INVALID_TOKEN,
        };
    };

    std::string current_line_;
    const std::string source_file_name_ = "<input>";
    // row and col of current input position.
    // Indexes is one based.
    // row_ is incremented after getline().
    // For example, do getline() that lineno is 1, row_++,
    // now row_ == 1 and then parse line that lineno is 1.
    size_t row_ = 0;
    // There is not col_ because column is derived from smatch.position().

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

            size_t current_line_head_pos = lines.size();

            {
                std::string s = stream_.getline();
                current_line_ = s;
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
                    auto token = create_token_from_match_result(match_result, current_line_head_pos);
                    assert(token.is_newline());
                    tokens.push_back(token);
                    continue;
                }
                if (match_result[sub_match_index::WHITESPACE_LINE].matched)
                {
                    // ignore it
                    break;
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
                    enqueue_all(std::move(tokens));
                    auto triple_quote_start_pos = match_result.position();
                    lines = lines.substr(triple_quote_start_pos);
                    break;
                }

                auto token = create_token_from_match_result(match_result, current_line_head_pos);
                tokens.push_back(token);
            } // end for

            if (has_continuous_line)
            {
                // ++it is still not end

                // continue while(true) until raw string closing quote (corespondind """ or ''') found
                continue;
            }

            assert(it == end || ++it == end); // ++it == end when after break for loop

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
        } // end while
    }

    Token create_token_from_match_result(const std::smatch &match_result, size_t current_line_head_pos)
    {

        if (has_sub_match<sub_match_index::STRING_LITERAL>(match_result))
        {
            auto prefix = match_result[sub_match_index::BYTES_LITERAL_PREFIX].str();
            auto contents = get_string_literal_contents(match_result);
            auto colmun_of_last_char = match_result.position() - current_line_head_pos;
            return Token::create_string_literal_token(
                match_result.str(),
                prefix,
                contents,
                get_current_source_location(colmun_of_last_char));
        }

        {
            auto index = get_first_sub_match_index(match_result);
            assert(index != 0);
            switch (index)
            {
            case sub_match_index::WHITESPACE_AT_BIGGINING_OF_LINE:
                return create_token<token_category::WHITESPACE_AT_BIGGINING_OF_LINE>(match_result);

            case sub_match_index::EMPTY_LINE:
            case sub_match_index::COMMENT:
            case sub_match_index::NEWLINE:
                return create_token<token_category::NEWLINE>(match_result);

            case sub_match_index::OPENING_BRACKET:
                return create_token<token_category::OPENING_BRACKET>(match_result);

            case sub_match_index::CLOSING_BRACKET:
                return create_token<token_category::CLOSING_BRACKET>(match_result);

            case sub_match_index::DEC_INTEGER_LITERAL:
                return create_integer_literal_token(10, match_result);

            case sub_match_index::BIN_INTEGER_LITERAL:
                return create_integer_literal_token(2, match_result);

            case sub_match_index::OCT_INTEGER_LITERAL:
                return create_integer_literal_token(8, match_result);

            case sub_match_index::HEX_INTEGER_LITERAL:
                return create_integer_literal_token(16, match_result);

            case sub_match_index::IDENTIFIER:
                return create_token<token_category::IDENTIFIER>(match_result);

            case sub_match_index::PYTHON_KEYWORD:
            case sub_match_index::DELIMITER:
                return create_token<token_category::ANY_OTHER>(match_result);

            case sub_match_index::INVALID_TOKEN:
            default:
                return Token::create_invalid_token(match_result.str(),
                                                   make_current_source_location(match_result),
                                                   "invalid token (phase 1 lexer)");
            }
        }
    }

    template <token_category C>
    Token create_token(const std::smatch &match_result) const
    {
        return Token::create_token<C>(match_result.str(), make_current_source_location(match_result));
    }

    Token create_integer_literal_token(size_t radix, const std::smatch &match_result) const
    {
        return Token::create_integer_literal_token(radix, match_result.str(), make_current_source_location(match_result));
    }

    static std::string get_string_literal_contents(const std::smatch &match_result)
    {
        if (has_sub_match<sub_match_index::TRIPLE_DOUBLE_QUOTED_CONTENTS>(match_result))
        {
            return match_result[sub_match_index::TRIPLE_DOUBLE_QUOTED_CONTENTS].str();
        }
        if (has_sub_match<sub_match_index::TRIPLE_SINGLE_QUOTED_CONTENTS>(match_result))
        {
            return match_result[sub_match_index::TRIPLE_SINGLE_QUOTED_CONTENTS].str();
        }
        if (has_sub_match<sub_match_index::DOUBLE_QUOTED_CONTENTS>(match_result))
        {
            return match_result[sub_match_index::DOUBLE_QUOTED_CONTENTS].str();
        }
        if (has_sub_match<sub_match_index::SINGLE_QUOTED_CONTENTS>(match_result))
        {
            return match_result[sub_match_index::SINGLE_QUOTED_CONTENTS].str();
        }
        assert(false && "never come here");
        return "";
    }

    template <size_t I>
    static bool has_sub_match(const std::smatch &match_result)
    {
        return match_result[I].matched;
    }


    static size_t get_first_sub_match_index(const std::smatch &match_result)
    {
        assert(match_result[0].matched);
        // not i = 0, match_result[0] is not a sub match.
        for (size_t i = 1; i < match_result.size(); ++i)
        {
            if (match_result[i].matched)
            {
                return i;
            }
        }
        std::cerr << "never come here, invalid match_result given\n";
        std::cerr << "match_result.str(): " << match_result.str() << "\n";
        assert(false && "never come here, invalid match_result given");
    }

    SourceLocation make_current_source_location(const std::smatch &match_result) const
    {
        return get_current_source_location(match_result.position());
    }

    /// zcolumn: zero based column position
    SourceLocation get_current_source_location(size_t zcolumn) const
    {
        return SourceLocation(source_file_name_, current_line_, one_based_index, row_, zcolumn + 1);
    }

    Token create_eof_token()
    {
        return Token::create_eof_token(get_current_source_location(current_line_.size()));
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

template <>
class Phase1TokenStream<std::stringstream> : public Phase1TokenStream<detail::StdStringStreamWrapper>
{
    using base_type = Phase1TokenStream<detail::StdStringStreamWrapper>;

public:
    using base_type::base_type;
};
} // namespace ljf::python

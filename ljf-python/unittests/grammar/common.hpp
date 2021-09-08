#pragma once

#include "gtest/gtest.h"

#include "ljf-python/tokenizer.hpp"
#include "ljf-python/grammar/expr.hpp"
#include "ljf-python/grammar/stmt.hpp"

using namespace ljf::python;
using namespace ljf::python::grammar;

using namespace ljf::python::parser;

static ExprGrammars<SStreamTokenStream> eg;
static StmtGrammars<SStreamTokenStream> sg;

template <typename Parser, typename TokenStream, typename... ErrArgs>
static auto parse_if(bool cond, const Parser &p, TokenStream &stream, ErrArgs &&...err_args)
{
    using result_content_ty = result_content_t<Parser, TokenStream>;
    if (cond)
    {
        return p(stream);
    }
    else
    {
        return make_error_result<result_content_ty>(stream.peek(), std::forward<ErrArgs>(err_args)...);
    }
}

template <typename Parser, typename TokenStream>
static auto parse_until_eof(const Parser &p, TokenStream &stream)
{
    auto parser = separator(many(newline)) + p + separator(many(newline)) + separator(eof);
    return parse_if(p.has_parser(), parser, stream, "parser not initialized");
}

template <typename TResult, typename F, typename TokenStream>
static auto parse_until_eof(const parser::Parser<TResult, F> &p, TokenStream &stream)
{
    auto parser = separator(many(newline)) + p + separator(many(newline)) + separator(eof);
    return parser(stream);
}

template <typename Parser>
static auto parse_until_end(const Parser &p, std::string input)
{
    SStreamTokenStream ts{input};
    return parse_until_eof(p, ts);
}

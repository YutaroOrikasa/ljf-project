#pragma once

#include "../grammar.hpp"
#include "expr.hpp"

#include "../ast/stmt.hpp"

namespace ljf::python::grammar
{

namespace StmtGrammars_
{
using namespace parser;
using namespace ast;

template <class TokenStream>
struct StmtGrammars : public ExprGrammars<TokenStream>
{
    template <typename T>
    using ParserPlaceHolder = parser::PlaceHolder<T, TokenStream>;
#define INIT_PLACE_HOLDER(name) \
    name{#name}
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(single_input);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(file_input);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(eval_input);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorator);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorators);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorated);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(async_funcdef);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(funcdef);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(parameters);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(typedargslist);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(tfpdef);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(varargslist);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(vfpdef);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(simple_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(small_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(expr_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(testlist_star_expr);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(augassign);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(del_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(pass_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(flow_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(break_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(continue_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(return_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(yield_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(raise_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_name);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_from);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_as_name);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_as_name);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_as_names);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_as_names);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_name);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(global_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(nonlocal_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(assert_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(compound_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(async_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(if_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(while_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(for_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(try_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(with_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(with_item);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(except_clause);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(suite);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(classdef);
};
} // namespace StmtGrammars_
using StmtGrammars_::StmtGrammars;
} // namespace ljf::python::grammar

#pragma once

#include "../grammar.hpp"
#include "expr.hpp"

#include "../ast/stmt.hpp"

namespace ljf::python::grammar {

namespace StmtGrammars_ {
    using namespace parser;
    using namespace ast;

    template <class TokenStream>
    struct StmtGrammars : public ExprGrammars<TokenStream> {
        template <typename T>
        using ParserPlaceHolder = parser::PlaceHolder<T, TokenStream>;
#define LJF_INIT_PLACE_HOLDER(name) name{#name}
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(single_input);
        ParserPlaceHolder<MultiStmt> LJF_INIT_PLACE_HOLDER(file_input);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(decorator);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(decorators);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(decorated);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(async_funcdef);
        ParserPlaceHolder<DefStmt> LJF_INIT_PLACE_HOLDER(funcdef);
        ParserPlaceHolder<FuncParams> LJF_INIT_PLACE_HOLDER(parameters);
        ParserPlaceHolder<FuncParams> LJF_INIT_PLACE_HOLDER(typedargslist);
        ParserPlaceHolder<IdentifierExpr> LJF_INIT_PLACE_HOLDER(tfpdef);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(varargslist);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(vfpdef);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(simple_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(small_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(expr_stmt);
        // ParserPlaceHolder<Stmt>
        // LJF_INIT_PLACE_HOLDER(testlist_star_expr);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(augassign);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(del_stmt);
        ParserPlaceHolder<PassStmt> LJF_INIT_PLACE_HOLDER(pass_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(flow_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(break_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(continue_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(return_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(yield_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(raise_stmt);
        ParserPlaceHolder<ImportStmt> LJF_INIT_PLACE_HOLDER(import_stmt);
        // ParserPlaceHolder<ImportStmt> LJF_INIT_PLACE_HOLDER(import_name);
        ParserPlaceHolder<ImportFrom> LJF_INIT_PLACE_HOLDER(import_from);
        ParserPlaceHolder<ImportAsName> LJF_INIT_PLACE_HOLDER(import_as_name);
        ParserPlaceHolder<DottedAsName> LJF_INIT_PLACE_HOLDER(dotted_as_name);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(import_as_names);
        // ParserPlaceHolder<ImportStmt>
        // LJF_INIT_PLACE_HOLDER(dotted_as_names); ParserPlaceHolder<Stmt>
        // LJF_INIT_PLACE_HOLDER(dotted_name); ParserPlaceHolder<Stmt>
        // LJF_INIT_PLACE_HOLDER(global_stmt); ParserPlaceHolder<Stmt>
        // LJF_INIT_PLACE_HOLDER(nonlocal_stmt); ParserPlaceHolder<Stmt>
        // LJF_INIT_PLACE_HOLDER(assert_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(compound_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(async_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(if_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(while_stmt);
        ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(for_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(try_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(with_stmt);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(with_item);
        // ParserPlaceHolder<Stmt> LJF_INIT_PLACE_HOLDER(except_clause);
        ParserPlaceHolder<MultiStmt> LJF_INIT_PLACE_HOLDER(suite);
        ParserPlaceHolder<ClassStmt> LJF_INIT_PLACE_HOLDER(classdef);

#undef LJF_INIT_PLACE_HOLDER

        StmtGrammars();
    };
} // namespace StmtGrammars_
using StmtGrammars_::StmtGrammars;
} // namespace ljf::python::grammar

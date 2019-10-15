#pragma once


#include "../grammar.hpp"
#include "expr.hpp"

#include "../ast/stmt.hpp"

namespace ljf::python::grammar
{

namespace detail
{

using namespace ast;
using namespace parser;

static constexpr auto fold_assign = [](auto &&first, auto &&right_list) -> Stmt {
    if (right_list.empty())
    {
        return ExprStmt(first);
    }

    auto right_hand_side = std::move(right_list.back());
    right_list.pop_back();

    std::vector<Expr> left_hand_side_list{std::move(first)};
    for (auto &&left : right_list)
    {
        left_hand_side_list.push_back(std::move(left));
    }
    return AssignStmt{
        std::move(left_hand_side_list),
        std::move(right_hand_side)};
};

template <typename T, typename P>
auto parser_result_type(P &&parser)
{
    return result_type<T> <<= parser;
}

template <typename T, typename P>
auto parser_result_brace(P &&parser)
{
    return result_type<T> <<= brace_init <<= parser;
}

namespace impl
{

template <typename T>
std::optional<T> flatten(std::optional<T> opt)
{
    return opt;
}

template <typename T>
std::optional<T> flatten(std::optional<std::optional<T>> opt_opt)
{
    if (!opt_opt)
    {
        return std::optional<T>();
    }

    return flatten(std::move(*opt_opt));
}

} // namespace impl

/// Flatten nested std::optional
inline constexpr auto flatten = [](auto &&data) {
    return impl::flatten(data);
};

inline constexpr auto flatten_conv = converter_no_strip(flatten);

inline constexpr auto flatten_parser = [](const auto &parser) {
    return flatten_conv <<= parser;
};

} // namespace detail

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
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(single_input);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(file_input);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorator);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorators);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(decorated);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(async_funcdef);
    ParserPlaceHolder<DefStmt> INIT_PLACE_HOLDER(funcdef);
    ParserPlaceHolder<FuncParams> INIT_PLACE_HOLDER(parameters);
    ParserPlaceHolder<FuncParams> INIT_PLACE_HOLDER(typedargslist);
    ParserPlaceHolder<IdentifierExpr> INIT_PLACE_HOLDER(tfpdef);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(varargslist);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(vfpdef);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(simple_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(small_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(expr_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(testlist_star_expr);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(augassign);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(del_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(pass_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(flow_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(break_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(continue_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(return_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(yield_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(raise_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_name);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_from);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_as_name);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_as_name);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(import_as_names);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_as_names);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(dotted_name);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(global_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(nonlocal_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(assert_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(compound_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(async_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(if_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(while_stmt);
    ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(for_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(try_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(with_stmt);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(with_item);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(except_clause);
    ParserPlaceHolder<MultiStmt> INIT_PLACE_HOLDER(suite);
    // ParserPlaceHolder<Stmt> INIT_PLACE_HOLDER(classdef);

    StmtGrammars()
    {
        using namespace detail;
        using namespace abbrev;

        using E = ExprGrammars<TokenStream>;

        // # Start symbols for the grammar:
        // #       single_input is a single interactive statement;
        // #       file_input is a module or sequence of commands read from an input file;
        // #       eval_input is the input for the eval() functions.
        // # NB: compound_stmt in single_input is followed by extra NEWLINE!
        // single_input = NEWLINE | simple_stmt | compound_stmt + NEWLINE;
        // file_input = (NEWLINE | stmt) * _many + ENDMARKER;

        // decorator = "@"_p + dotted_name + opt["("_p + opt[E::arglist] + ")"] + NEWLINE;
        // decorators = decorator * _many1;
        // decorated = decorators + (classdef | funcdef | async_funcdef);
        // async_funcdef = "async"_p + funcdef;

        // Simplified funcdef definition (return type annotation is omitted)
        funcdef = brace_init <<= "def"_sep + NAME + parameters + ":"_sep + suite;
        // funcdef = "def"_sep + NAME + parameters + opt["->"_sep + E::test] + ":"_sep + suite;

        parameters = "("_sep + opt[typedargslist] + ")"_sep;
        const Parser defparameter = result_type<DefParameter> <<= brace_init <<= tfpdef + opt["="_sep + E::test];
        // int x = defparameter;
        const Parser starred_param = result_type<StarredParameter> <<= brace_init <<= "*"_sep + tfpdef;

        const Parser stars = result_type<StarredParams> <<=
            parser_result_brace<ast::StarredParameter>("*"_sep + tfpdef) //
            + opt[","_sep + parser_result_brace<ast::DoubleStarredParameter>("**"_sep + tfpdef)];

        // Simplified typedargslist definition
        // This parser does NOT accept such things:
        //      def f(*,a):pass
        //      def f(*a,b):pass
        //      def f(*a,b, **c):pass
        typedargslist = result_type<FuncParams> <<=
            (defparameter + (","_sep + defparameter) * _many //
             + flatten_parser(opt[","_sep + opt[stars]]))    //
            | stars                                          //
            | parser_result_brace<ast::DoubleStarredParameter>("**"_sep + tfpdef);
        // typedargslist = (defparameter + (","_sep + defparameter) * _many              //
        //                      + opt[","_sep + opt[("*"_sep + opt[tfpdef]               //
        //                                           + (","_sep + defparameter) * _many  //
        //                                           + opt[","_sep + "**"_sep + tfpdef]) //
        //                                          | "**"_sep + tfpdef]]                //
        //                  | "*"_sep + opt[tfpdef]                                      //
        //                        + (","_sep + defparameter) * _many                     //
        //                        + opt[","_sep + "**"_sep + tfpdef]                     //
        //                  | "**"_sep + tfpdef);

        // Simplified tfpdef definition (type annotation is omitted)
        tfpdef = NAME;
        // tfpdef = NAME + opt[":"_p + E::test];

        // varargslist = (vfpdef + opt["="_p + E::test] + (","_p + vfpdef + opt["="_p + E::test]) * _many + opt[","_p + opt["*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + E::test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef]] | "*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + E::test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef);
        // vfpdef = NAME;
        stmt = simple_stmt | compound_stmt;

        // Simplified small_stmt definition
        simple_stmt = small_stmt + sep(NEWLINE);
        // simple_stmt = printer("simple_stmt") + small_stmt + (";"_p + small_stmt) * _many + opt[";"] + NEWLINE;

        // Simplified small_stmt definition
        small_stmt = expr_stmt;
        // small_stmt = printer("small_stmt") + (del_stmt | pass_stmt | flow_stmt |
        //                                       import_stmt | global_stmt | nonlocal_stmt | assert_stmt | expr_stmt);

        // Simplified expr_stmt definition
        expr_stmt = converter(fold_assign) <<= E::exprlist + ("="_sep + E::exprlist) * _many;
        // expr_stmt = testlist_star_expr + (augassign + (E::yield_expr | E::testlist) |
        //                                   ("="_p + (E::yield_expr | testlist_star_expr)) * _many);
        // augassign = ("+="_p | "-="_p | "*="_p | "@="_p | "/="_p | "%="_p | "&="_p | "|="_p | "^="_p |
        //              "<<="_p | ">>="_p | "**="_p | "//=");
        // // # For normal assignments, additional restrictions enforced by the interpreter
        // del_stmt = "del"_p + E::exprlist;
        // pass_stmt = "pass"_p;
        // flow_stmt = break_stmt | continue_stmt | return_stmt | raise_stmt | yield_stmt;
        // break_stmt = "break"_p;
        // continue_stmt = "continue"_p;
        // return_stmt = "return"_p + opt[E::testlist];
        // yield_stmt = E::yield_expr;
        // raise_stmt = "raise"_p + opt[E::test + opt["from"_p + E::test]];

        // import_stmt = import_name | import_from;
        // import_name = "import"_p + dotted_as_names;
        // // # note below = the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
        // import_from = ("from"_p + (("."_p | "...") * _many + dotted_name | ("."_p | "...") * _many1) + "import"_p + ("*"_p | "("_p + import_as_names + ")"_p | import_as_names));
        // import_as_name = NAME + opt["as"_p + NAME];
        // dotted_as_name = dotted_name + opt["as"_p + NAME];
        // import_as_names = import_as_name + (","_p + import_as_name) * _many + opt[","];
        // dotted_as_names = dotted_as_name + (","_p + dotted_as_name) * _many;
        // dotted_name = NAME + ("."_p + NAME) * _many;
        // global_stmt = "global"_p + NAME + (","_p + NAME) * _many;
        // nonlocal_stmt = "nonlocal"_p + NAME + (","_p + NAME) * _many;
        // assert_stmt = "assert"_p + E::test + opt[","_p + E::test];

        // Simplified expr_stmt definition
        compound_stmt = if_stmt;
        // compound_stmt = printer("compound_stmt") + if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated | async_stmt;
        // async_stmt = "async"_p + (funcdef | with_stmt | for_stmt);
        if_stmt = result_type<IfStmt> <<= brace_init <<= "if"_sep + E::test + ":"_sep + suite +
                                                         (result_type<ast::Elif> <<= brace_init <<= "elif"_sep + E::test + ":"_sep + suite) * _many +
                                                         opt["else"_sep + ":"_sep + suite];
        // while_stmt = "while"_p + E::test + ":"_p + suite + opt["else"_p + ":"_p + suite];

        // Simplified for_stmt definition, omit else caluse
        for_stmt = result_type<ForStmt> <<= brace_init <<= "for"_sep + E::exprlist + "in"_sep + E::testlist + ":"_sep + suite;
        // for_stmt = "for"_sep + E::exprlist + "in"_sep + E::testlist + ":"_sep + suite + opt["else"_sep + ":"_p + suite];

        // try_stmt = ("try"_p + ":"_p + suite + ((except_clause + ":"_p + suite) * _many1 + opt["else"_p + ":"_p + suite] + opt["finally"_p + ":"_p + suite] | "finally"_p + ":"_p + suite));
        // with_stmt = "with"_p + with_item + (","_p + with_item) * _many + ":"_p + suite;
        // with_item = E::test + opt["as"_p + E::expr];
        // // # NB compile.c makes sure that the default except clause is last
        // except_clause = "except"_p + opt[E::test + opt["as"_p + NAME]];

        suite = simple_stmt | sep(NEWLINE) + prompter("(please indent)> ") +
                                  sep(INDENT) +
                                  (stmt + prompter("(in indent)> ")) * _many1_unify +
                                  sep(DEDENT);

        // classdef = "class"_p + NAME + opt["("_p + opt[E::arglist] + ")"] + ":"_p + suite;
    }
};
} // namespace StmtGrammars_
using StmtGrammars_::StmtGrammars;
} // namespace ljf::python::grammar

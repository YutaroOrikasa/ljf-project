#pragma once

#include "../grammar.hpp"

namespace ljf::python::grammar
{
namespace ExprGrammars_
{
using namespace parser;
using namespace ast;

template <class TokenStream>
struct ExprGrammars
{
    template <typename T>
    using ParserPlaceHolder = parser::PlaceHolder<T, TokenStream>;
#define INIT_PLACE_HOLDER(name) \
    name { #name }

    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(eval_input);

    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(test);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(test_nocond);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(lambdef);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(lambdef_nocond);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(or_test);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(and_test);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(not_test);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(comparison);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(comp_op);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(star_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(xor_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(and_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(shift_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(arith_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(term);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(factor);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(power);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(atom_expr);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(atom);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(testlist_comp);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(trailer);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(subscriptlist);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(subscript);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(sliceop);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(exprlist);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(testlist);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(dictorsetmaker);
    ParserPlaceHolder<ArgList> INIT_PLACE_HOLDER(arglist);
    ParserPlaceHolder<Argument> INIT_PLACE_HOLDER(argument);
    ParserPlaceHolder<CompIter> INIT_PLACE_HOLDER(comp_iter);
    ParserPlaceHolder<CompFor> INIT_PLACE_HOLDER(comp_for);
    ParserPlaceHolder<CompIf> INIT_PLACE_HOLDER(comp_if);
    ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(encoding_decl);
    ParserPlaceHolder<YieldExpr> INIT_PLACE_HOLDER(yield_expr);
    // ParserPlaceHolder<Expr> INIT_PLACE_HOLDER(yield_arg);
#undef INIT_PLACE_HOLDER

    ExprGrammars();
};
} // namespace ExprGrammars_

using ExprGrammars_::ExprGrammars;

inline parser::PlaceHolder<ast::Expr, IStreamTokenStream> make_python_eval_input_parser()
{
    ExprGrammars<IStreamTokenStream> expr_grammars;
    return expr_grammars.testlist;
}
} // namespace ljf::python::grammar

#include "expr-impl.hpp"

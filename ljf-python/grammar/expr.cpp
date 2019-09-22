#include "../grammar.hpp"

namespace ljf::python::parser
{
ParserPlaceHolder<SExpr> make_python_eval_input_parser()
{
    using namespace impl;
#define INIT_PLACE_HOLDER(name) name{#name}

    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(eval_input);

    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(test);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(test_nocond);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(lambdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(lambdef_nocond);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(or_test);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(and_test);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(not_test);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comparison);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_op);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(star_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(xor_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(and_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(shift_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(arith_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(term);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(factor);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(power);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(atom_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(atom);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(testlist_comp);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(trailer);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(subscriptlist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(subscript);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(sliceop);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(exprlist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(testlist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(dictorsetmaker);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(arglist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(argument);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_iter);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_for);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_if);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(encoding_decl);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(yield_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(yield_arg);
#undef INIT_PLACE_HOLDER

    const Parser vfpdef = NAME;
    const Parser varargslist = (vfpdef + opt["="_p + test] + (","_p + vfpdef + opt["="_p + test]) * _many                                                                   //
                                    + opt[","_p + opt["*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef]] //
                                | "*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef);

    test = or_test + opt["if"_p + or_test + "else"_p + test] | lambdef;
    test_nocond = or_test | lambdef_nocond;
    lambdef = "lambda"_p + opt[varargslist] + ":"_p + test;
    lambdef_nocond = "lambda"_p + opt[varargslist] + ":"_p + test_nocond;
    or_test = and_test + ("or"_p + and_test) * _many;
    and_test = not_test + ("and"_p + not_test) * _many;
    not_test = "not"_p + not_test | comparison;
    comparison = expr + (comp_op + expr) * _many;
    // # <> isn't actually a valid comparison operator in Python. It's here for the
    // # sake of a __future__ import described in PEP 401 (which really works :-)
    comp_op = "<"_p | ">"_p | "=="_p | ">="_p | "<="_p | "<>"_p | "!="_p | "in"_p | "not"_p + "in"_p | "is"_p | "is"_p + "not";
    star_expr = "*"_p + expr;
    expr = xor_expr + ("|"_p + xor_expr) * _many;
    xor_expr = and_expr + ("^"_p + and_expr) * _many;
    and_expr = shift_expr + ("&"_p + shift_expr) * _many;
    shift_expr = arith_expr + (("<<"_p | ">>") + arith_expr) * _many;
    arith_expr = term + (("+"_p | "-") + term) * _many;
    term = factor + (("*"_p | "@"_p | "/"_p | "%"_p | "//") + factor) * _many;
    factor = ("+"_p | "-"_p | "~") + factor | power;
    power = atom_expr + opt["**"_p + factor];
    atom_expr = opt["await"] + atom + trailer * _many;
    atom = ("("_p + opt[yield_expr | testlist_comp] + ")"_p |
            "["_p + opt[testlist_comp] + "]"_p |
            "{"_p + opt[dictorsetmaker] + "}"_p |
            NAME | NUMBER | STRING * _many1 | "..."_p | "None"_p | "True"_p | "False");
    testlist_comp = (test | star_expr) + (comp_for | (","_p + (test | star_expr)) * _many + opt[","]);
    trailer = "("_p + opt[arglist] + ")"_p | "["_p + subscriptlist + "]"_p | "."_p + NAME;
    subscriptlist = subscript + (","_p + subscript) * _many + opt[","];
    subscript = test | opt[test] + ":"_p + opt[test] + opt[sliceop];
    sliceop = ":"_p + opt[test];
    exprlist = (expr | star_expr) + (","_p + (expr | star_expr)) * _many + opt[","];
    testlist = test + (","_p + test) * _many + opt[","];
    dictorsetmaker = (((test + ":"_p + test | "**"_p + expr) + (comp_for | (","_p + (test + ":"_p + test | "**"_p + expr)) * _many + opt[","])) |
                      ((test | star_expr) + (comp_for | (","_p + (test | star_expr)) * _many + opt[","])));

    arglist = argument + (","_p + argument) * _many + opt[","];

    // # The reason that keywords are test nodes instead of NAME is that using NAME
    // # results in an ambiguity. ast.c makes sure it's a NAME.
    // # "test '=' test"_pis really "keyword '=' test", but we have no such token.
    // # These need to be in a single rule to avoid grammar that is ambiguous
    // # to our LL(1) parser. Even though 'test' includes '*expr' in star_expr,
    // # we explicitly match '*' here, too, to give it proper precedence.
    // # Illegal combinations and orderings are blocked in ast.c:
    // # multiple (test comp_for) arguments are blocked; keyword unpackings
    // # that precede iterable unpackings are blocked; etc.
    argument = (test + opt[comp_for] |
                test + "="_p + test |
                "**"_p + test |
                "*"_p + test);

    comp_iter = comp_for | comp_if;
    comp_for = "for"_p + exprlist + "in"_p + or_test + opt[comp_iter];
    comp_if = "if"_p + test_nocond + opt[comp_iter];

    // # not used in grammar, but may appear in "node" passed from Parser to Compiler
    encoding_decl = NAME;

    yield_expr = "yield"_p + opt[yield_arg];
    yield_arg = "from"_p + test | testlist;

    eval_input = testlist + NEWLINE * _many + ENDMARKER;
    return testlist;
}
} // namespace ljf::python::parser

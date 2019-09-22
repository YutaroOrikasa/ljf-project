#include "../grammar.hpp"

namespace ljf::python::parser
{
ParserPlaceHolder<SExpr> make_python_grammer_parser()
{
    using namespace impl;

#define INIT_PLACE_HOLDER(name) name{#name}

    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(single_input);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(file_input);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(eval_input);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(decorator);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(decorators);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(decorated);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(async_funcdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(funcdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(parameters);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(typedargslist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(tfpdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(varargslist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(vfpdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(simple_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(small_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(expr_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(testlist_star_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(augassign);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(del_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(pass_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(flow_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(break_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(continue_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(return_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(yield_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(raise_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(import_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(import_name);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(import_from);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(import_as_name);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(dotted_as_name);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(import_as_names);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(dotted_as_names);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(dotted_name);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(global_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(nonlocal_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(assert_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(compound_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(async_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(if_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(while_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(for_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(try_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(with_stmt);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(with_item);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(except_clause);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(suite);
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
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(classdef);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(arglist);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(argument);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_iter);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_for);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(comp_if);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(encoding_decl);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(yield_expr);
    ParserPlaceHolder<SExpr> INIT_PLACE_HOLDER(yield_arg);

#undef INIT_PLACE_HOLDER

    // # Start symbols for the grammar:
    // #       single_input is a single interactive statement;
    // #       file_input is a module or sequence of commands read from an input file;
    // #       eval_input is the input for the eval() functions.
    // # NB: compound_stmt in single_input is followed by extra NEWLINE!
    single_input = NEWLINE | simple_stmt | compound_stmt + NEWLINE;
    file_input = (NEWLINE | stmt) * _many + ENDMARKER;
    eval_input = testlist + NEWLINE * _many + ENDMARKER;

    decorator = "@"_p + dotted_name + opt["("_p + opt[arglist] + ")"] + NEWLINE;
    decorators = decorator * _many1;
    decorated = decorators + (classdef | funcdef | async_funcdef);
    async_funcdef = "async"_p + funcdef;
    funcdef = "def"_p + printer("funcdef") + NAME + parameters + opt["->"_p + test] + ":"_p + suite;

    parameters = "("_p + opt[typedargslist] + ")";
    typedargslist = (tfpdef + opt["="_p + test] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + opt["*"_p + opt[tfpdef] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + tfpdef] | "**"_p + tfpdef]] | "*"_p + opt[tfpdef] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + tfpdef] | "**"_p + tfpdef);
    tfpdef = NAME + opt[":"_p + test];
    varargslist = (vfpdef + opt["="_p + test] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + opt["*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef]] | "*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef);
    vfpdef = NAME;
    stmt = simple_stmt | compound_stmt;
    simple_stmt = printer("simple_stmt") + small_stmt + (";"_p + small_stmt) * _many + opt[";"] + NEWLINE;
    small_stmt = printer("small_stmt") + (del_stmt | pass_stmt | flow_stmt |
                                          import_stmt | global_stmt | nonlocal_stmt | assert_stmt | expr_stmt);
    expr_stmt = testlist_star_expr + (augassign + (yield_expr | testlist) |
                                      ("="_p + (yield_expr | testlist_star_expr)) * _many);
    testlist_star_expr = (test | star_expr) + (","_p + (test | star_expr)) * _many + opt[","];
    augassign = ("+="_p | "-="_p | "*="_p | "@="_p | "/="_p | "%="_p | "&="_p | "|="_p | "^="_p |
                 "<<="_p | ">>="_p | "**="_p | "//=");
    // # For normal assignments, additional restrictions enforced by the interpreter
    del_stmt = "del"_p + exprlist;
    pass_stmt = "pass"_p;
    flow_stmt = break_stmt | continue_stmt | return_stmt | raise_stmt | yield_stmt;
    break_stmt = "break"_p;
    continue_stmt = "continue"_p;
    return_stmt = "return"_p + opt[testlist];
    yield_stmt = yield_expr;
    raise_stmt = "raise"_p + opt[test + opt["from"_p + test]];
    import_stmt = import_name | import_from;
    import_name = "import"_p + dotted_as_names;
    // # note below = the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
    import_from = ("from"_p + (("."_p | "...") * _many + dotted_name | ("."_p | "...") * _many1) + "import"_p + ("*"_p | "("_p + import_as_names + ")"_p | import_as_names));
    import_as_name = NAME + opt["as"_p + NAME];
    dotted_as_name = dotted_name + opt["as"_p + NAME];
    import_as_names = import_as_name + (","_p + import_as_name) * _many + opt[","];
    dotted_as_names = dotted_as_name + (","_p + dotted_as_name) * _many;
    dotted_name = NAME + ("."_p + NAME) * _many;
    global_stmt = "global"_p + NAME + (","_p + NAME) * _many;
    nonlocal_stmt = "nonlocal"_p + NAME + (","_p + NAME) * _many;
    assert_stmt = "assert"_p + test + opt[","_p + test];

    compound_stmt = printer("compound_stmt") + if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated | async_stmt;
    async_stmt = "async"_p + (funcdef | with_stmt | for_stmt);
    if_stmt = "if"_p + test + ":"_p + suite + ("elif"_p + test + ":"_p + suite) * _many + opt["else"_p + ":"_p + suite];
    while_stmt = "while"_p + test + ":"_p + suite + opt["else"_p + ":"_p + suite];
    for_stmt = "for"_p + exprlist + "in"_p + testlist + ":"_p + suite + opt["else"_p + ":"_p + suite];
    try_stmt = ("try"_p + ":"_p + suite + ((except_clause + ":"_p + suite) * _many1 + opt["else"_p + ":"_p + suite] + opt["finally"_p + ":"_p + suite] | "finally"_p + ":"_p + suite));
    with_stmt = "with"_p + with_item + (","_p + with_item) * _many + ":"_p + suite;
    with_item = test + opt["as"_p + expr];
    // # NB compile.c makes sure that the default except clause is last
    except_clause = "except"_p + opt[test + opt["as"_p + NAME]];
    suite = simple_stmt | NEWLINE + (prompter("(please indent)> ") + INDENT) + (stmt + prompter("(in indent)> ")) * _many1 + DEDENT + printer("<DEDENT>");

    classdef = "class"_p + NAME + opt["("_p + opt[arglist] + ")"] + ":"_p + suite;

    // --- expressions ---
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

    return single_input;
}
} // namespace ljf::python::parser

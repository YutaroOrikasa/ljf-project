

#pragma once

// copyid from https://github.com/python/cpython/blob/3.5/Grammar/Grammar

// This file is C++ port of python grammer file.

// # Grammar for Python

// # Note:  Changing the grammar specified in this file will most likely
// #        require corresponding changes in the parser module
// #        (../Modules/parsermodule.c).  If you can't make the changes to
// #        that module yourself, please co-ordinate the required changes
// #        with someone who can; ask around on python-dev for help.  Fred
// #        Drake <fdrake@acm.org> will probably be listening there.

// # NOTE WELL: You should also follow all the steps listed at
// # https://docs.python.org/devguide/grammar.html

#include <type_traits>
#include <variant>
#include <vector>

#include "parser.hpp"

namespace ljf::python::parser
{

class SExpr
{
private:
    using SExprList = std::vector<SExpr>;
    std::variant<std::monostate, Token, SExprList, ast::Expr> variant_;

public:
    template <typename... Args, std::enable_if_t<sizeof...(Args) >= 2> * = nullptr>
    SExpr(Args &&...) {}

    SExpr() = default;
    // SExpr(const SExpr&) = delete;
    // SExpr(SExpr&& other) : variant_(std::move(other.variant_)) {};
    // SExpr& operator=(const SExpr&) = delete;
    // SExpr& operator=(SExpr&&other) {
    //     variant_ =std::move(other.variant_);
    //     return *this;
    // }

    SExpr(const SExpr &) = default;
    SExpr(SExpr &&other) = default;
    SExpr &operator=(const SExpr &) = default;
    SExpr &operator=(SExpr &&other) = default;

    explicit SExpr(Token token) : variant_(std::in_place_type<Token>, std::move(token)) {}

    explicit SExpr(ast::Expr expr) : variant_(std::in_place_type<ast::Expr>, std::move(expr)) {}
};

auto printer(const std::string &str)
{
    return Parser([str](const auto &token_steram) {
        (void)token_steram;
        std::cerr << str << "\n";
        return Separator();
    });
}

namespace impl
{
struct Opt
{
    template <typename Parser>
    constexpr auto operator[](Parser &&p) const noexcept
    {
        return option(std::forward<Parser>(p));
    }

    template <size_t N>
    constexpr auto operator[](const char (&str)[N]) const noexcept
    {
        return option_str(str);
    }
};

template <size_t>
struct Many
{
};

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<0>)
{
    return many(std::forward<Parser>(p));
}

template <typename Parser>
constexpr auto operator*(Parser &&p, Many<1>)
{
    return p + many(p);
}

inline constexpr Opt opt;

inline constexpr Many<0> _many;
inline constexpr Many<1> _many1;

} // namespace impl

inline auto make_python_grammer_parser()
{
    using namespace impl;

    ParserPlaceHolder<SExpr> single_input;
    ParserPlaceHolder<SExpr> file_input;
    ParserPlaceHolder<SExpr> eval_input;
    ParserPlaceHolder<SExpr> decorator;
    ParserPlaceHolder<SExpr> decorators;
    ParserPlaceHolder<SExpr> decorated;
    ParserPlaceHolder<SExpr> async_funcdef;
    ParserPlaceHolder<SExpr> funcdef;
    ParserPlaceHolder<SExpr> parameters;
    ParserPlaceHolder<SExpr> typedargslist;
    ParserPlaceHolder<SExpr> tfpdef;
    ParserPlaceHolder<SExpr> varargslist;
    ParserPlaceHolder<SExpr> vfpdef;
    ParserPlaceHolder<SExpr> stmt;
    ParserPlaceHolder<SExpr> simple_stmt;
    ParserPlaceHolder<SExpr> small_stmt;
    ParserPlaceHolder<SExpr> expr_stmt;
    ParserPlaceHolder<SExpr> testlist_star_expr;
    ParserPlaceHolder<SExpr> augassign;
    ParserPlaceHolder<SExpr> del_stmt;
    ParserPlaceHolder<SExpr> pass_stmt;
    ParserPlaceHolder<SExpr> flow_stmt;
    ParserPlaceHolder<SExpr> break_stmt;
    ParserPlaceHolder<SExpr> continue_stmt;
    ParserPlaceHolder<SExpr> return_stmt;
    ParserPlaceHolder<SExpr> yield_stmt;
    ParserPlaceHolder<SExpr> raise_stmt;
    ParserPlaceHolder<SExpr> import_stmt;
    ParserPlaceHolder<SExpr> import_name;
    ParserPlaceHolder<SExpr> import_from;
    ParserPlaceHolder<SExpr> import_as_name;
    ParserPlaceHolder<SExpr> dotted_as_name;
    ParserPlaceHolder<SExpr> import_as_names;
    ParserPlaceHolder<SExpr> dotted_as_names;
    ParserPlaceHolder<SExpr> dotted_name;
    ParserPlaceHolder<SExpr> global_stmt;
    ParserPlaceHolder<SExpr> nonlocal_stmt;
    ParserPlaceHolder<SExpr> assert_stmt;
    ParserPlaceHolder<SExpr> compound_stmt;
    ParserPlaceHolder<SExpr> async_stmt;
    ParserPlaceHolder<SExpr> if_stmt;
    ParserPlaceHolder<SExpr> while_stmt;
    ParserPlaceHolder<SExpr> for_stmt;
    ParserPlaceHolder<SExpr> try_stmt;
    ParserPlaceHolder<SExpr> with_stmt;
    ParserPlaceHolder<SExpr> with_item;
    ParserPlaceHolder<SExpr> except_clause;
    ParserPlaceHolder<SExpr> suite;
    ParserPlaceHolder<SExpr> test;
    ParserPlaceHolder<SExpr> test_nocond;
    ParserPlaceHolder<SExpr> lambdef;
    ParserPlaceHolder<SExpr> lambdef_nocond;
    ParserPlaceHolder<SExpr> or_test;
    ParserPlaceHolder<SExpr> and_test;
    ParserPlaceHolder<SExpr> not_test;
    ParserPlaceHolder<SExpr> comparison;
    ParserPlaceHolder<SExpr> comp_op;
    ParserPlaceHolder<SExpr> star_expr;
    ParserPlaceHolder<SExpr> expr;
    ParserPlaceHolder<SExpr> xor_expr;
    ParserPlaceHolder<SExpr> and_expr;
    ParserPlaceHolder<SExpr> shift_expr;
    ParserPlaceHolder<SExpr> arith_expr;
    ParserPlaceHolder<SExpr> term;
    ParserPlaceHolder<SExpr> factor;
    ParserPlaceHolder<SExpr> power;
    ParserPlaceHolder<SExpr> atom_expr;
    ParserPlaceHolder<SExpr> atom;
    ParserPlaceHolder<SExpr> testlist_comp;
    ParserPlaceHolder<SExpr> trailer;
    ParserPlaceHolder<SExpr> subscriptlist;
    ParserPlaceHolder<SExpr> subscript;
    ParserPlaceHolder<SExpr> sliceop;
    ParserPlaceHolder<SExpr> exprlist;
    ParserPlaceHolder<SExpr> testlist;
    ParserPlaceHolder<SExpr> dictorsetmaker;
    ParserPlaceHolder<SExpr> classdef;
    ParserPlaceHolder<SExpr> arglist;
    ParserPlaceHolder<SExpr> argument;
    ParserPlaceHolder<SExpr> comp_iter;
    ParserPlaceHolder<SExpr> comp_for;
    ParserPlaceHolder<SExpr> comp_if;
    ParserPlaceHolder<SExpr> encoding_decl;
    ParserPlaceHolder<SExpr> yield_expr;
    ParserPlaceHolder<SExpr> yield_arg;

    const Parser NEWLINE = newline;
    const Parser ENDMARKER = eof;
    const Parser NAME = identifier;
    const Parser INDENT = indent;
    const Parser DEDENT = dedent;
    const Parser NUMBER = integer_literal;
    const Parser STRING = string_literal;

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
    funcdef = printer("funcdef") + "def"_p + NAME + parameters + opt["->"_p + test] + ":"_p + suite;
    assert(funcdef.has_parser());

    parameters = "("_p + opt[typedargslist] + ")";
    typedargslist = (tfpdef + opt["="_p + test] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + opt["*"_p + opt[tfpdef] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + tfpdef] | "**"_p + tfpdef]] | "*"_p + opt[tfpdef] + (","_p + tfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + tfpdef] | "**"_p + tfpdef);
    tfpdef = NAME + opt[":"_p + test];
    varargslist = (vfpdef + opt["="_p + test] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + opt["*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef]] | "*"_p + opt[vfpdef] + (","_p + vfpdef + opt["="_p + test]) * _many + opt[","_p + "**"_p + vfpdef] | "**"_p + vfpdef);
    vfpdef = NAME;
    stmt = simple_stmt | compound_stmt;
    simple_stmt = small_stmt + (";"_p + small_stmt) * _many + opt[";"] + NEWLINE;
    small_stmt = (expr_stmt | del_stmt | pass_stmt | flow_stmt |
                  import_stmt | global_stmt | nonlocal_stmt | assert_stmt);
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

    compound_stmt = if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | printer("call funcdef") + funcdef | classdef | decorated | async_stmt;
    async_stmt = "async"_p + (funcdef | with_stmt | for_stmt);
    if_stmt = "if"_p + test + ":"_p + suite + ("elif"_p + test + ":"_p + suite) * _many + opt["else"_p + ":"_p + suite];
    while_stmt = "while"_p + test + ":"_p + suite + opt["else"_p + ":"_p + suite];
    for_stmt = "for"_p + exprlist + "in"_p + testlist + ":"_p + suite + opt["else"_p + ":"_p + suite];
    try_stmt = ("try"_p + ":"_p + suite + ((except_clause + ":"_p + suite) * _many1 + opt["else"_p + ":"_p + suite] + opt["finally"_p + ":"_p + suite] | "finally"_p + ":"_p + suite));
    with_stmt = "with"_p + with_item + (","_p + with_item) * _many + ":"_p + suite;
    with_item = test + opt["as"_p + expr];
    // # NB compile.c makes sure that the default except clause is last
    except_clause = "except"_p + opt[test + opt["as"_p + NAME]];
    suite = simple_stmt | NEWLINE + INDENT + stmt * _many1 + DEDENT;

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

    classdef = "class"_p + NAME + opt["("_p + opt[arglist] + ")"] + ":"_p + suite;

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

    // # not used in grammar, but may appear in "node"_ppassed from ParserPlaceHolder<SExpr> to Compiler
    encoding_decl = NAME;

    yield_expr = "yield"_p + opt[yield_arg];
    yield_arg = "from"_p + test | testlist;

    return single_input;
}

} // namespace ljf::python::parser

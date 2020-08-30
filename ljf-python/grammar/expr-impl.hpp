#pragma once

#include "expr.hpp"

namespace ljf::python::grammar
{
namespace detail
{

using parser::detail::fold_left_to_vec;

using namespace ast;
using namespace parser;
static constexpr auto fold_left = [](auto &&first, auto &&vec) -> Expr {
    Expr e0 = first;

    for (auto &&[oper, right] : vec)
    {
        auto oper_token = make_from_variant<Token>(oper);
        e0 = BinaryExpr(e0, oper_token, right);
    }
    return e0;
};

static constexpr auto fold_left_opt = [](auto &&first, auto &&opt) -> Expr {
    Expr e0 = first;

    if (opt)
    {
        auto &&[oper, right] = *opt;
        auto oper_token = make_from_variant<Token>(oper);
        e0 = BinaryExpr(e0, oper_token, right);
    }
    return e0;
};

template <typename Comp = Comprehension, typename T>
static std::variant<std::vector<T>, Comp>
fold_to_vector_or_comprehension(T t,
                                std::variant<CompFor, std::vector<T>> rest)
{
    if (auto comp_for = std::get_if<CompFor>(&rest))
    {
        return Comp{std::move(t), std::move(*comp_for)};
    }
    else
    {
        return fold_left_to_vec(std::move(t), std::move(std::get<1>(rest)));
    }
};

static constexpr auto fold_to_exprlist_or_comprehension =
    [](Expr expr,
       std::variant<CompFor, std::vector<Expr>> rest)
    -> std::variant<ExprList, Comprehension> {
    return fold_to_vector_or_comprehension(std::move(expr), std::move(rest));
};

static constexpr auto make_expr_or_comprehension =
    [](Expr expr,
       std::optional<CompFor> opt_comp_for)
    -> std::variant<Expr, Comprehension> {
    if (opt_comp_for)
    {
        return Comprehension{std::move(expr), std::move(*opt_comp_for)};
    }

    return expr;
};

static constexpr auto fold_to_key_datum_list_or_comprehension =
    [](auto key_datum,
       auto rest) {
        return fold_to_vector_or_comprehension(std::move(key_datum), std::move(rest));
    };

namespace
{
template <typename EnclosureExpr, typename ComprehensionExpr, typename ElemExpr = ast::Expr>
struct DisplayExprFactory
{
    Expr operator()(std::vector<ElemExpr> expr_list) const
    {
        static_assert(!std::is_same_v<EnclosureExpr, TupleExpr>);
        return EnclosureExpr(std::move(expr_list));
    }

    Expr operator()(Comprehension comp) const
    {
        return ComprehensionExpr(std::move(comp));
    }

    Expr operator()() const
    {
        return EnclosureExpr();
    }
};

struct ParenthFormExprFactory : public DisplayExprFactory<TupleExpr, GeneratorExpr>
{
    using DisplayExprFactory::operator();

    Expr operator()(std::vector<Expr> expr_list) const
    {
        if (expr_list.size() == 1)
        {
            return expr_list[0];
        }

        return TupleExpr(std::move(expr_list));
    }
    Expr operator()(YieldExpr expr) const
    {
        return expr;
    }
};
} // namespace

static constexpr auto parenth_form_conv = converter(ParenthFormExprFactory());

static constexpr auto list_display_conv = converter(DisplayExprFactory<ListExpr, ListComprehensionExpr>());

template <typename T>
static constexpr auto default_type = [](auto opt) {
    using optional_content_type = std::decay_t<decltype(*opt)>;

    using return_variant_type = std::variant<T, optional_content_type>;
    if (!opt)
    {
        return return_variant_type(std::in_place_index<0>, T());
    }
    else
    {
        return return_variant_type(std::in_place_index<1>, std::move(*opt));
    }
};

// // dict_or_set
// static Expr dict_or_set_display_ctor_impl()
// {
//     return DictExpr();
// }

// static Expr dict_or_set_display_ctor_impl(Comprehension comp)
// {
//     return ListComprehensionExpr(std::move(comp));
// }

// static Expr dict_or_set_display_ctor_impl(std::vector<Expr> expr_list)
// {
//     return SetExpr(std::move(expr_list));
// }

// static constexpr auto dict_or_set_display_ctor = [](auto &&... args) -> Expr {
//     return dict_or_set_display_ctor_impl(std::move(args)...);
// };

// static constexpr auto dict_or_set_display_conv = converter(dict_or_set_display_ctor);

static constexpr auto dict_display_conv = converter(DisplayExprFactory<DictExpr, DictComprehensionExpr, DictKeyValueExpr>());

// string
static constexpr auto concat_str_exprs =
    [](StringLiteralExpr str_expr, std::vector<StringLiteralExpr> rest) -> Result<StringLiteralExpr> {
    if (rest.size() != 0)
    {
        return make_error(rest[0].token(), "unsupported feature: string literal concatnation");
    }
    return success_move(str_expr);
};

// expr list
static constexpr auto expr_list_ctor = [](Expr first, std::vector<Expr> rest, auto &&opt_comma) -> Expr {
    // matches such like:
    //   a
    //   a + b
    if (rest.empty() && !opt_comma)
    {

        return first;
    }

    // matches such like:
    //   a,
    //   a, b
    //   a, b,
    return TupleExpr(fold_left_to_vec(std::move(first), std::move(rest)));
};

static constexpr auto combine_tokens = [](Token token1, Token token2) -> Token {
    auto combined_str = token1.str() + " " + token2.str();
    return Token::create_token<token_category::ANY_OTHER>(combined_str, token1.source_location());
};

} // namespace detail

template <class TokenStream>
inline ExprGrammars<TokenStream>::ExprGrammars()
{
    using namespace abbrev;
    using namespace detail;
    using ast::Expr;

    const Parser vfpdef = NAME;
    const Parser defparameter = vfpdef + opt["="_sep + test];
    const Parser parameter_list_starargs = "*"_p + opt[vfpdef]                  //
                                               + (","_p + defparameter) * _many //
                                               + opt[","_p + "**"_p + vfpdef]   //
                                           | "**"_p + vfpdef;
    const Parser varargslist = (defparameter + (","_sep + defparameter) * _many   //
                                    + opt[","_sep + opt[parameter_list_starargs]] //
                                | parameter_list_starargs);

    and_test = converter(fold_left) <<= not_test + ("and"_p + not_test) * _many;
    or_test = converter(fold_left) <<= and_test + ("or"_p + and_test) * _many;
    not_test = (result_type<UnaryExpr> <<= "not"_p + not_test) | comparison;

    test = (result_type<ConditionalExpr> <<= or_test + opt["if"_sep + or_test + "else"_sep + test]) | lambdef;
    test_nocond = or_test | lambdef_nocond;
    auto to_lambda_dummy = [](auto &&, Expr) {
        return LambdaExpr();
    };
    lambdef = converter(to_lambda_dummy) <<= "lambda"_sep + opt[varargslist] + ":"_sep + test;
    lambdef_nocond = converter(to_lambda_dummy) <<= "lambda"_sep + opt[varargslist] + ":"_sep + test_nocond;

    expr = converter(fold_left) <<= xor_expr + ("|"_p + xor_expr) * _many;

    auto combine_tokens_conv = converter(combine_tokens);
    // # <> isn't actually a valid comparison operator in Python. It's here for the
    // # sake of a __future__ import described in PEP 401 (which really works :-)
    const Parser comp_op = "<"_p | ">"_p | "=="_p | ">="_p | "<="_p | "<>"_p | "!="_p //
                           | "in"_p                                                   //
                           | (combine_tokens_conv <<= "not"_p + "in"_p)               //
                           | "is"_p                                                   //
                           | (combine_tokens_conv <<= "is"_p + "not"_p);
    comparison = converter(fold_left) <<= expr + (comp_op + expr) * _many;
    star_expr = result_type<StarExpr> <<= "*"_sep + expr;
    xor_expr = converter(fold_left) <<= and_expr + ("^"_p + and_expr) * _many;
    and_expr = converter(fold_left) <<= shift_expr + ("&"_p + shift_expr) * _many;
    shift_expr = converter(fold_left) <<= arith_expr + (("<<"_p | ">>") + arith_expr) * _many;
    arith_expr = converter(fold_left) <<= term + (("+"_p | "-") + term) * _many;
    term = converter(fold_left) <<= factor + (("*"_p | "@"_p | "/"_p | "%"_p | "//") + factor) * _many;
    factor = (result_type<UnaryExpr> <<= (result_type<Token> <<= "+"_p | "-"_p | "~") + factor) | power;
    power = converter(fold_left_opt) <<= atom_expr + opt["**"_p + factor];

    arglist = converter(fold_left_to_vec) <<= argument + (","_sep + argument) * _many + sep(opt[","]);
    const Parser subscriptlist = converter(fold_left_to_vec) <<= subscript + (","_sep + subscript) * _many + sep(opt[","]);
    const Parser trailer = result_type<Trailer> <<=
        "("_sep + opt[arglist] + ")"_sep    //
        | "["_sep + subscriptlist + "]"_sep //
        | (result_type<DotIdentifier> <<= "."_sep + NAME);

    const Parser testlist_comp = converter(fold_to_exprlist_or_comprehension) <<=
        (result_type<Expr> <<= test | star_expr) + (comp_for | (","_sep + (result_type<Expr> <<= test | star_expr)) * _many + sep(opt[","]));
    atom_expr = result_type<AtomExpr> <<= opt["await"] + atom + trailer * _many;

    atom = ((parenth_form_conv <<= "("_sep + opt[yield_expr | testlist_comp] + ")"_sep)                //
            | (list_display_conv <<= "["_sep + opt[testlist_comp] + "]"_sep)                           //
            | (converter_no_strip(default_type<DictExpr>) <<= "{"_sep + opt[dictorsetmaker] + "}"_sep) //
            | NAME | NUMBER | (converter(concat_str_exprs) <<= STRING * _many1)                        //
            | (result_type<BuiltinObjectExpr> <<= "..."_p | "None"_p | "True"_p | "False"));

    const Parser sliceop = ":"_sep + opt[test];
    subscript = test | (result_type<SliceExpr> <<= opt[test] + ":"_sep + opt[test] + opt[sliceop]);

    const Parser expr_or_star_expr = result_type<Expr> <<= expr | star_expr;
    exprlist = converter(expr_list_ctor) <<= expr_or_star_expr + (","_sep + expr_or_star_expr) * _many + opt[","];
    testlist = converter(expr_list_ctor) <<= test + (","_sep + test) * _many + opt[","];
    const Parser key_datum = result_type<DictKeyValueExpr> <<= test + ":"_sep + test | "**"_p + expr;
    const Parser dict_maker = dict_display_conv <<= converter(fold_to_key_datum_list_or_comprehension) <<=
        (key_datum + (comp_for | (","_sep + key_datum) * _many + sep(opt[","])));
    const Parser set_maker = testlist_comp;
    // dictorsetmaker = (dict_maker | set_maker);
    dictorsetmaker = (dict_maker);

    // # The reason that keywords are test nodes instead of NAME is that using NAME
    // # results in an ambiguity. ast.c makes sure it's a NAME.
    // # "test '=' test" is really "keyword '=' test", but we have no such token.
    // # These need to be in a single rule to avoid grammar that is ambiguous
    // # to our LL(1) parser. Even though 'test' includes '*expr' in star_expr,
    // # we explicitly match '*' here, too, to give it proper precedence.
    // # Illegal combinations and orderings are blocked in ast.c:
    // # multiple (test comp_for) arguments are blocked; keyword unpackings
    // # that precede iterable unpackings are blocked; etc.
    argument = ((converter(make_expr_or_comprehension) <<= test + opt[comp_for]) |
                (test + "="_p + test)
                // srared arg syntaxes are omitted.
                /* |
                 "**"_p + test |
                "*"_p + test */
    );

    comp_iter = comp_for | comp_if;
    comp_for = "for"_sep + exprlist + "in"_sep + or_test + opt[comp_iter];
    comp_if = "if"_sep + test_nocond + opt[comp_iter];

    // # not used in grammar, but may appear in "node" passed from Parser to Compiler
    encoding_decl = NAME;

    const Parser yield_arg = "from"_p + test | testlist;
    yield_expr = "yield"_sep + opt[yield_arg];

    eval_input = testlist + sep(NEWLINE * _many) + sep(ENDMARKER);
}
} // namespace ljf::python::grammar

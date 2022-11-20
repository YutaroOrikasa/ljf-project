#include "stmt.hpp"

namespace ljf::python::grammar
{
namespace detail
{

using namespace ast;
using namespace parser;

static constexpr auto fold_assign = [](auto &&first, auto &&right_list) -> Stmt
{
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

/// return type: std::vector<decltype(first)>
static constexpr auto unify_many1 = [](auto first, auto vec)
{
    std::vector<decltype(first)> ret_vec{std::move(first)};

    for (auto &&elem : vec)
    {
        ret_vec.push_back(std::move(elem));
    }
    return ret_vec;
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
inline constexpr auto flatten = [](auto &&data)
{
    return impl::flatten(data);
};

inline constexpr auto flatten_conv = converter_no_strip(flatten);

inline constexpr auto flatten_parser = [](const auto &parser)
{
    return flatten_conv <<= parser;
};

/// equivalent to optional(p + many(sep + p) + optional(sep))
/// result type: vector<result of p>
template <typename Parser, typename SepParser>
constexpr auto many_sep_end_by(Parser p, SepParser sep)
{
    // discard pair[1]
    auto conv = converter([](auto pair)
                          {
                              auto [vec, ends_with_sep] = std::move(pair);
                              return vec;
                          });
    return conv <<= sep_many_optsep(p, sep);
}

} // namespace detail

template <class TokenStream>
inline StmtGrammars<TokenStream>::StmtGrammars()
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
    constexpr auto discard_separator_from_raw_file_input = //
        [](auto &&raw_file_input_vec)
    {
        std::vector<ast::Stmt> result;
        result.reserve(raw_file_input_vec.size());
        for (auto &&newline_or_stmt : raw_file_input_vec)
        {
            if (auto *ast = std::get_if<ast::Stmt>(&newline_or_stmt))
            {
                result.push_back(*ast);
            }
        }
        return result;
    };

    file_input = converter(discard_separator_from_raw_file_input) //
        <<= (separator(NEWLINE) | stmt) * _many + separator(ENDMARKER);

    // decorator = "@"_p + dotted_name + opt["("_p + opt[E::arglist] + ")"] + NEWLINE;
    // decorators = decorator * _many1;
    // decorated = decorators + (classdef | funcdef | async_funcdef);
    // async_funcdef = "async"_p + funcdef;

    // Simplified funcdef definition (return type annotation is omitted)
    funcdef = brace_init <<= "def"_sep + NAME + parameters + ":"_sep + suite;
    // funcdef = "def"_sep + NAME + parameters + opt["->"_sep + E::test] + ":"_sep + suite;

    parameters = "("_sep + opt[typedargslist] + ")"_sep;
    const Parser defparameter = result_type<DefParameter> <<= brace_init <<= tfpdef + opt["="_sep + E::test];
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
        detail::sep_many_optsep_optend(defparameter, ","_sep, stars);
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
    small_stmt = pass_stmt | flow_stmt | import_stmt | expr_stmt;
    // small_stmt = (del_stmt | pass_stmt | flow_stmt |
    //                                       import_stmt | global_stmt | nonlocal_stmt | assert_stmt | expr_stmt);

    // Simplified expr_stmt definition
    expr_stmt = converter(fold_assign) <<= E::exprlist + ("="_sep + E::exprlist) * _many;
    // expr_stmt = testlist_star_expr + (augassign + (E::yield_expr | E::testlist) |
    //                                   ("="_p + (E::yield_expr | testlist_star_expr)) * _many);
    // augassign = ("+="_p | "-="_p | "*="_p | "@="_p | "/="_p | "%="_p | "&="_p | "|="_p | "^="_p |
    //              "<<="_p | ">>="_p | "**="_p | "//=");
    // // # For normal assignments, additional restrictions enforced by the interpreter
    // del_stmt = "del"_p + E::exprlist;
    pass_stmt = result_type<PassStmt> <<= "pass"_p;

    // Simplified flow_stmt definition
    flow_stmt = break_stmt | continue_stmt | return_stmt;
    // flow_stmt = break_stmt | continue_stmt | return_stmt | raise_stmt | yield_stmt;
    break_stmt = result_type<FlowStmt> <<= "break"_p;
    continue_stmt = result_type<FlowStmt> <<= "continue"_p;
    return_stmt = result_type<ReturnStmt> <<= "return"_sep + opt[E::testlist];
    // yield_stmt = E::yield_expr;
    // raise_stmt = "raise"_p + opt[E::test + opt["from"_p + E::test]];

    auto dotted_name = converter(unify_many1) <<= NAME + ("."_sep + NAME) * _many;
    auto dotted_as_names = converter(unify_many1) <<= dotted_as_name + (","_sep + dotted_as_name) * _many;
    auto import_name = "import"_sep + dotted_as_names;
    // ImportStmt is struct but do not use brace_init because funny compile error occurs...
    import_stmt = /* brace_init <<= */ import_name | import_from;

    // Simplified import_as_names definition
    // I think trailing [,] is not necessary
    auto import_as_names = converter(unify_many1) <<= import_as_name + (","_sep + import_as_name) * _many;
    // auto import_as_names = converter(unify_many1) <<= import_as_name + (","_sep + import_as_name) * _many + opt[","];

    struct ImportFromFromPart
    {
        size_t dot_num = 0;
        std::vector<IdentifierExpr> from_names;
    };

    const auto normalize_dot_or_elipsis = [](const std::variant<Token, Token> &dot_or_elipsis) -> size_t
    {
        if (auto dot = std::get_if<0>(&dot_or_elipsis))
        {
            return 1;
        }
        else
        {
            auto elipsis = std::get<1>(dot_or_elipsis);
            assert(elipsis == "...");
            return 3;
        }
    };

    const auto import_from_from_part = [dotted_name, normalize_dot_or_elipsis](auto &&token_stream) -> Result<ImportFromFromPart>
    {
        size_t dot_num = 0;
        auto dot_or_elipsis_parser = converter_no_strip(normalize_dot_or_elipsis) <<= "."_p | "..."_p;
        auto first_result = LL1_parse(dot_or_elipsis_parser, token_stream);
        if (first_result.fatally_failed())
        {
            return move_to_another_error_result<ImportFromFromPart>(first_result.result);
        }
        if (first_result.result)
        {
            dot_num += first_result.result.success();
            while (true)
            {
                auto many_dot_or_elipsis_result = LL1_parse(dot_or_elipsis_parser, token_stream);
                if (many_dot_or_elipsis_result.fatally_failed())
                {
                    return move_to_another_error_result<ImportFromFromPart>(many_dot_or_elipsis_result.result);
                }
                if (many_dot_or_elipsis_result.result.failed())
                {
                    auto dotted_name_result = LL1_parse(dotted_name, token_stream);
                    if (dotted_name_result.fatally_failed())
                    {
                        return move_to_another_error_result<ImportFromFromPart>(many_dot_or_elipsis_result.result);
                    }
                    if (dotted_name_result.result.failed())
                    {
                        ImportFromFromPart content{dot_num, {}};
                        return Result<ImportFromFromPart>(content);
                    }
                    ImportFromFromPart content{dot_num, dotted_name_result.result.extract_success()};
                    return Result<ImportFromFromPart>(content);
                }
                dot_num += many_dot_or_elipsis_result.result.success();
            }
        }
        else
        {
            auto dotted_name_result = dotted_name(token_stream);
            if (dotted_name_result.failed())
            {
                return move_to_another_error_result<ImportFromFromPart>(dotted_name_result);
            }
            ImportFromFromPart content{0, dotted_name_result.extract_success()};
            return Result<ImportFromFromPart>(content);
        }
    };

    struct ImportFromImportPart
    {
        std::variant<ImportFrom::Wildcard, std::vector<ImportAsName>> wildcard_or_import_as_names;
        ImportFromImportPart(Token arg) : wildcard_or_import_as_names(ImportFrom::Wildcard())
        {
            assert(arg == "*");
        }
        ImportFromImportPart(std::vector<ImportAsName> arg) : wildcard_or_import_as_names(arg) {}
    };

    const auto to_import_from = [](const auto &a) -> ImportFrom
    {
        const std::tuple<ImportFromFromPart, ImportFromImportPart> &b = a;
        auto [from, import] = b;

        return ImportFrom{from.dot_num, from.from_names, import.wildcard_or_import_as_names};
    };
    // # note below = the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
    // Do not use converter() insted of converter_no_strip() because funny compile error occurs...
    import_from = converter_no_strip(to_import_from) <<= ("from"_sep + import_from_from_part //
                                                          + "import"_sep + (result_type<ImportFromImportPart> <<= "*"_p | "("_sep + import_as_names + ")"_sep | import_as_names));

    import_as_name = brace_init <<= NAME + opt["as"_sep + NAME];
    dotted_as_name = brace_init <<= dotted_name + opt["as"_sep + NAME];

    // global_stmt = "global"_p + NAME + (","_p + NAME) * _many;
    // nonlocal_stmt = "nonlocal"_p + NAME + (","_p + NAME) * _many;
    // assert_stmt = "assert"_p + E::test + opt[","_p + E::test];

    // Simplified expr_stmt definition
    compound_stmt = if_stmt | for_stmt | funcdef | classdef;
    // compound_stmt = if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated | async_stmt;
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

    const auto to_class_stmt = [](const auto &arg)
    {
        auto &[name, opt_inheritance_list, body] = arg;
        ArgList inheritance_list;
        if (opt_inheritance_list.has_value())
        {
            inheritance_list = *opt_inheritance_list;
        }
        return ClassStmt{name, inheritance_list, body};
    };
    classdef = converter_no_strip(to_class_stmt) <<= "class"_sep + NAME + flatten_parser(opt["("_sep + opt[E::arglist] + ")"_sep]) + ":"_sep + suite;
}
} // namespace ljf::python::grammar

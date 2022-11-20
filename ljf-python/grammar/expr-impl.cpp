#include "expr-impl.hpp"
#include "../tokenizer.hpp"

namespace ljf::python::grammar::ExprGrammars_ {
template struct ExprGrammars<IStreamTokenStream>;
template struct ExprGrammars<FStreamTokenStream>;
template struct ExprGrammars<SStreamTokenStream>;
} // namespace ljf::python::grammar::ExprGrammars_

#include "../tokenizer.hpp"
#include "stmt-impl.hpp"
namespace ljf::python::grammar::StmtGrammars_
{
template struct StmtGrammars<IStreamTokenStream>;
template struct StmtGrammars<FStreamTokenStream>;
template struct StmtGrammars<SStreamTokenStream>;
} // namespace ljf::python::grammar::StmtGrammars_

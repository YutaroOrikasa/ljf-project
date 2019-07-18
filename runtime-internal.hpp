#pragma once

#include <ljf/runtime.hpp>

namespace llvm
{
    class Function;
} // namespace llvm


namespace ljf
{
    LJFFunctionId register_llvm_function(llvm::Function* f);
} // namespace ljf

namespace ljf::internal
{
    Environment* create_environment();
}

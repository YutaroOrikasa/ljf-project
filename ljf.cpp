
#include <dlfcn.h>

#include <stdexcept>
#include <string>
#include <iostream>

#include "ljf/ljf.hpp"
#include "runtime/runtime-internal.hpp"

using namespace std::literals::string_literals;

namespace
{
constexpr auto DLOPEN_LJF_RUNTIME_FLAGS = RTLD_LAZY | RTLD_GLOBAL;
} // namespace

namespace ljf
{
static ljf_internal_initialize_t ljf_internal_initialize;

static void load_ljf_runtime(const std::string &runtime_filename)
{
    // try finding runtime symbol in this program
    auto this_program_handle = dlopen(NULL, RTLD_LAZY);
    if (!this_program_handle)
    {
        throw std::runtime_error(std::string("dlopen(NULL, RTLD_LAZY) failed: ") + dlerror());
    }
    auto ljf_internal_initialize_addr = dlsym(this_program_handle, "ljf_internal_initialize");
    if (ljf_internal_initialize_addr)
    {
        throw std::logic_error("ljf::initialize(): ljf runtime already loaded."
                               " ljf runtime must not be linked to your program. "
                               " ljf runtime must be loaded by dlopen() in ljf::initialize().");
        ljf_internal_initialize = reinterpret_cast<ljf_internal_initialize_t>(ljf_internal_initialize_addr);
    }
    else
    {
        auto rt_handle = dlopen(runtime_filename.c_str(), DLOPEN_LJF_RUNTIME_FLAGS);
        if (!rt_handle)
        {
            throw std::invalid_argument("dlopen ljf runtime \"" + runtime_filename + "\" failed (dlopen): " + dlerror());
        }
        auto ljf_internal_initialize_addr = dlsym(rt_handle, "ljf_internal_initialize");
        if (!ljf_internal_initialize_addr)
        {
            throw std::runtime_error("internal runtime function not found (dlsym): "s + dlerror());
        }
        ljf_internal_initialize = reinterpret_cast<ljf_internal_initialize_t>(ljf_internal_initialize_addr);
    }
}

void initialize(const CompilerMap &compiler_map, const std::string &ljf_tmpdir, const std::string &runtime_filename)
{

    load_ljf_runtime(runtime_filename);
    assert(ljf_internal_initialize);

    ljf_internal_initialize(compiler_map, ljf_tmpdir, runtime_filename);
}
} // namespace ljf

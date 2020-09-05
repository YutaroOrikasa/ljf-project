
#include <dlfcn.h>

#include <stdexcept>
#include <string>
#include <iostream>

#include "ljf/ljf.hpp"
#include "runtime/runtime-internal.hpp"

// runtime.so must be loaded with dlopen().
// runtime.so must not be statically linked to main executable.
// runtime.so must not be dynamically linked to main executable.
//
// The reason why runtime.so must not be statically linked
// is JIT compiled code (.so) can not be linked to ljf runtime functoins.
//
// The reason why runtime.so must not be dynamically linked
// is there are no portable methods that we know the path
// to the dynamic link library (this case, the path to runtime.so)
// that is linked to running process itself.

using namespace std::literals::string_literals;

namespace
{
constexpr auto DLOPEN_LJF_RUNTIME_FLAGS = RTLD_LAZY | RTLD_GLOBAL;
} // namespace

namespace ljf
{
static ljf_internal_initialize_t ljf_internal_initialize;
static ljf_internal_start_entry_point_t ljf_internal_start_entry_point;

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

        auto ljf_internal_start_entry_point_addr = dlsym(rt_handle, "ljf_internal_start_entry_point");
        if (!ljf_internal_start_entry_point_addr)
        {
            throw std::runtime_error("internal runtime function not found (dlsym): "s + dlerror());
        }
        ljf_internal_start_entry_point = reinterpret_cast<ljf_internal_start_entry_point_t>(ljf_internal_start_entry_point_addr);
    }
}

void initialize(const CompilerMap &compiler_map, const std::string &ljf_tmpdir, const std::string &runtime_filename)
{

    load_ljf_runtime(runtime_filename);
    assert(ljf_internal_initialize);

    ljf_internal_initialize(compiler_map, ljf_tmpdir, runtime_filename);
}

int start_entry_point_of_source(const std::string &language, const std::string &source_path,
                                int argc, const char **argv)
{
    return ljf_internal_start_entry_point(nullptr, language, source_path, argc, argv);
}

int start_entry_point_of_function_ptr(ljf_main_t ljf_main, int argc, const char **argv)
{
    return ljf_internal_start_entry_point(ljf_main, "", "", argc, argv);
}

int start_entry_point_of_native_dynamic_library(std::string dynamic_library_path, int argc, const char **argv)
{
    // load
    auto handle = dlopen(dynamic_library_path.c_str(), RTLD_LAZY);
    if (!handle)
    {
        throw std::invalid_argument(std::string("loading ") + dynamic_library_path + " failed (dlopen): " + dlerror());
    }
    auto ljf_module_init_addr = dlsym(handle, "ljf_module_init");
    if (!ljf_module_init_addr)
    {
        throw std::invalid_argument(std::string("loading ") + dynamic_library_path + " failed (dlsym): " + dlerror());
    }

    reinterpret_cast<void (*)()>(ljf_module_init_addr)();

    auto addr = dlsym(handle, "ljf_main");
    if (!addr)
    {
        throw std::invalid_argument(std::string("loading ") + dynamic_library_path + " failed (dlsym): " + dlerror());
    }

    return start_entry_point_of_function_ptr(reinterpret_cast<ljf_main_t>(addr), argc, argv);
}
} // namespace ljf

#include <stdint.h>
#include <iostream>

#include <ljf/runtime.hpp>

#include "common.hpp"

void object_new(uint64_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        ljf_new_object();
    }
}

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{
    auto n = (1 << 23);

    {
        std::cout << "-- object_new --" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        object_new(n);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    return env;
}

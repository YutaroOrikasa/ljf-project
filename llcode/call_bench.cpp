#include <stdint.h>
#include <iostream>

#include <ljf/runtime.hpp>

#include "common.hpp"

extern "C" LJFObject *fun(LJFObject *env, LJFObject *tmp)
{
    return ljf_undefined;
    return ljf_get_object_from_environment(env, "a");
}

void call_bench(LJFObject *env, uint64_t n)
{
    auto fn_obj = ljf_get_object_from_environment(env, "fun");
    auto fn_env = ljf_get_object_from_hidden_table(fn_obj, "env");
    auto fn_id = ljf_get_function_id_from_function_table(fn_obj, "call");
    for (size_t i = 0; i < n; i++)
    {
        // auto fn_obj = ljf_get_object_from_environment(env, "fun");
        // auto fn_env = ljf_get_object_from_hidden_table(fn_obj, "env");
        // auto fn_id = ljf_get_function_id_from_function_table(fn_obj, "call");
        // auto args = ljf_new_object();
        // ljf_set_object_to_environment(env, "tmp.args", args);
        // ljf_set_object_to_table(args, "a", ljf_undefined);
        // ljf_set_object_to_table(args, "b", true_);
        // ljf_set_object_to_table(args, "c", false_);
        // ljf_call_function(fn_id, fn_env, args);

        ljf_call_function(fn_id, fn_env, nullptr);
    }
}

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{
    add_func_obj_to_env(env, module_func_table, "fun");
    auto n = (1 << 23);

    {
        std::cout << "-- call_bench --" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        call_bench(env, n);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    return env;
}

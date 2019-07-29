#include <stdint.h>
#include <iostream>
#include <atomic>

#include <ljf/runtime.hpp>

#include "common.hpp"

extern "C" LJFObject *add_bench_ljf_closure(LJFObject *env, LJFObject *tmp)
{
    // while (a <= n)
    // {
    //     a = a + ONE;
    // }
    // return int64_t(a);
    // std::cout << "**a: " << ljf_get_native_data(ljf_get_object_from_environment(env, "a")) << "\n";

    auto a = ljf_get_object_from_environment(env, "a");
    auto a_class = ljf_get_object_from_table(a, "class");
    auto opLe = ljf_get_function_id_from_function_table(a_class, "<=");
    auto n = ljf_get_object_from_environment(env, "n");
    auto a_LE_n = [&]() {
        auto args = ljf_new_object();
        ljf_push_object_to_array(tmp, args);
        ljf_set_object_to_table(args, "a", a);
        ljf_set_object_to_table(args, "b", n);
        auto class_env = ljf_get_object_from_hidden_table(a_class, "env");
        auto r = ljf_call_function(opLe, class_env, args);
        ljf_push_object_to_array(tmp, r);
        return r;
    }();
    if (a_LE_n == false_)
    {
        // std::cout << "*a: " << ljf_get_native_data(ljf_get_object_from_environment(env, "a")) << "\n";
        // std::cout << "*n: " << ljf_get_native_data(ljf_get_object_from_environment(env, "n")) << "\n";
        return break_;
    }

    {
        auto a = ljf_get_object_from_environment(env, "a");
        auto a_class = ljf_get_object_from_table(a, "class");
        auto class_env = ljf_get_object_from_hidden_table(a_class, "env");
        auto opAdd = ljf_get_function_id_from_function_table(a_class, "+");

        auto args = ljf_new_object();
        ljf_push_object_to_array(tmp, args);
        ljf_set_object_to_table(args, "a", a);
        auto ONE = ljf_get_object_from_environment(env, "ONE");
        ljf_set_object_to_table(args, "b", ONE);
        auto a2 = ljf_call_function(opAdd, class_env, args);
        ljf_set_object_to_environment(env, "a", a2);

        // std::cout << "*a: " << ljf_get_native_data(ljf_get_object_from_environment(env, "a")) << "\n";
        // std::cout << "*n: " << ljf_get_native_data(ljf_get_object_from_environment(env, "n")) << "\n";
        return continue_;
    }
}

extern "C" LJFObject *add_bench_ljf(LJFObject *env, LJFObject *tmp)
{
    auto const1 =
        [&]() {
            auto Int_class = ljf_get_object_from_environment(env, "Int");
            auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
            auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
            auto const1 = ljf_new_object_with_native_data(1);
            ljf_push_object_to_array(tmp, const1);
            auto args = ljf_new_object();
            ljf_push_object_to_array(tmp, args);
            ljf_set_object_to_table(args, "self", const1);
            ljf_call_function(Int_init, Int_class_env, args);
            return const1;
        }();
    ljf_set_object_to_environment(env, "ONE", const1);

    auto const0 =
        [&]() {
            auto Int_class = ljf_get_object_from_environment(env, "Int");
            auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
            auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
            auto const0 = ljf_new_object_with_native_data(0);
            ljf_push_object_to_array(tmp, const0);
            auto args = ljf_new_object();
            ljf_push_object_to_array(tmp, args);
            ljf_set_object_to_table(args, "self", const0);
            ljf_call_function(Int_init, Int_class_env, args);
            return const0;
        }();
    ljf_set_object_to_environment(env, "a", const0);
    while (true)
    {
        auto fn_obj = ljf_get_object_from_environment(env, "add_bench_ljf_closure");
        auto fn_id = ljf_get_function_id_from_function_table(fn_obj, "call");
        auto controle = ljf_call_function(fn_id, env, nullptr);
        // std::cout << "a: " << ljf_get_native_data(ljf_get_object_from_environment(env, "a")) << "\n";
        if (controle == break_)
        {
            break;
        }
    }
    return ljf_get_object_from_environment(env, "a");
}

std::atomic<int64_t> add_bench(std::atomic<int64_t> &n)
{
    std::atomic<int64_t> ONE = 1;
    std::atomic<int64_t> a = 0;
    while (a <= n)
    {
        a = a + ONE;
    }
    return int64_t(a);
}

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{
    initIntClass(env, module_func_table);
    {
        auto add_bench_ljf_id = ljf_get_function_id_from_function_table(module_func_table, "add_bench_ljf");
        auto add_bench_ljf_obj = ljf_new_object();
        ljf_push_object_to_array(env, add_bench_ljf_obj);
        ljf_set_function_id_to_function_table(add_bench_ljf_obj, "call", add_bench_ljf_id);
        ljf_set_object_to_table(add_bench_ljf_obj, "env", env);
        ljf_set_object_to_environment(env, "add_bench_ljf", add_bench_ljf_obj);
    }
    {
        auto add_bench_ljf_closure_id = ljf_get_function_id_from_function_table(module_func_table, "add_bench_ljf_closure");
        auto add_bench_ljf_closure_obj = ljf_new_object();
        ljf_push_object_to_array(env, add_bench_ljf_closure_obj);
        ljf_set_function_id_to_function_table(add_bench_ljf_closure_obj, "call", add_bench_ljf_closure_id);
        ljf_set_object_to_table(add_bench_ljf_closure_obj, "env", env);
        ljf_set_object_to_environment(env, "add_bench_ljf_closure", add_bench_ljf_closure_obj);
    }
    std::atomic<int64_t> n = (1 << 17);

    {
        std::cout << "-- add_bench --" << std::endl;
        mygperf::ProfilerStart("tmp/add_bench.prof");
        auto start = std::chrono::high_resolution_clock::now();
        auto r = add_bench(n);
        auto end = std::chrono::high_resolution_clock::now();
        mygperf::ProfilerStop();
        auto elapsed = end - start;
        std::cout << "add_bench(" << n << ") = " << r << std::endl;
        std::cout << "elapsed ms (native) " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    {
        std::cout << "-- add_bench_ljf --" << std::endl;
        auto args = ljf_new_object();
        ljf_push_object_to_array(env, args);
        ljf_set_object_to_table(args, "n", newInt(env, env, n));
        mygperf::ProfilerStart("tmp/add_bench-ljf.prof");
        auto start = std::chrono::high_resolution_clock::now();
        auto add_bench_ljf_obj = ljf_get_object_from_environment(env, "add_bench_ljf");
        auto add_bench_ljf_env = ljf_get_object_from_table(add_bench_ljf_obj, "env");
        auto add_bench_ljf = ljf_get_function_id_from_function_table(add_bench_ljf_obj, "call");
        auto r = ljf_call_function(add_bench_ljf, add_bench_ljf_env, args);
        ljf_push_object_to_array(env, r);
        auto end = std::chrono::high_resolution_clock::now();
        mygperf::ProfilerStop();
        auto elapsed = end - start;
        std::cout << "add_bench(" << n << ") = " << ljf_get_native_data(r) << std::endl;
        std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    return env;
}

#include <stdint.h>
#include <iostream>

#include <ljf/runtime.hpp>

#include "common.hpp"

int64_t tarai(int64_t x, int64_t y, int64_t z)
{
    if (x <= y)
    {
        return y;
    }
    else
    {
        return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
    }
}

extern "C" LJFObject *tarai_ljf(LJFObject *env, LJFObject *tmp)
{
    // x <= y
    auto x = ljf_get_object_from_environment(env, "x");
    auto x_class = ljf_get_object_from_table(x, "class");
    auto opLe = ljf_get_function_id_from_function_table(x_class, "<=");
    auto y = ljf_get_object_from_environment(env, "y");
    auto tmp0 = [&]() {
        auto args = ljf_new_object();
        ljf_push_object_to_array(tmp, args);
        ljf_set_object_to_table(args, "a", x);
        ljf_set_object_to_table(args, "b", y);
        auto class_env = ljf_get_object_from_hidden_table(x_class, "env");
        auto r = ljf_call_function(opLe, class_env, args);
        ljf_push_object_to_array(tmp, r);
        return r;
    }();
    if (tmp0 == true_)
    {
        return y;
    }
    else
    {
        // tarai(x - 1, y, z)
        auto result1 =
            [&]() {
                // x - 1
                auto v = ljf_get_object_from_environment(env, "x");
                auto v_class = ljf_get_object_from_table(v, "class");
                auto class_env = ljf_get_object_from_hidden_table(v_class, "env");
                auto opSub = ljf_get_function_id_from_function_table(v_class, "-");

                // 1
                LJFObject *const1;
                {
                    auto Int_class = ljf_get_object_from_environment(env, "Int");
                    auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
                    auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
                    const1 = ljf_new_object_with_native_data(1);
                    ljf_push_object_to_array(tmp, const1);
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "self", const1);
                    ljf_call_function(Int_init, Int_class_env, args);
                }
                // x - 1
                LJFObject *v2;
                {
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "a", v);
                    ljf_set_object_to_table(args, "b", const1);
                    v2 = ljf_call_function(opSub, class_env, args);
                    ljf_push_object_to_array(tmp, v2);
                }

                auto x = v2;
                auto y = ljf_get_object_from_environment(env, "y");
                auto z = ljf_get_object_from_environment(env, "z");
                auto args = ljf_new_object();
                ljf_push_object_to_array(tmp, args);
                ljf_set_object_to_table(args, "y", y);
                ljf_set_object_to_table(args, "z", z);
                ljf_set_object_to_table(args, "x", x);
                auto tarai_ljf_obj = ljf_get_object_from_environment(env, "tarai_ljf");
                auto tarai_ljf_env = ljf_get_object_from_table(tarai_ljf_obj, "env");
                auto tarai_ljf = ljf_get_function_id_from_function_table(tarai_ljf_obj, "call");
                // tarai(x - 1, y, z)
                auto result = ljf_call_function(tarai_ljf, tarai_ljf_env, args);
                ljf_push_object_to_array(tmp, result);
                return result;
            }();

        // tarai(y - 1, z, x)
        auto result2 =
            [&]() {
                // y - 1
                auto v = ljf_get_object_from_environment(env, "y");
                auto v_class = ljf_get_object_from_table(v, "class");
                auto class_env = ljf_get_object_from_hidden_table(v_class, "env");
                auto opSub = ljf_get_function_id_from_function_table(v_class, "-");

                // 1
                LJFObject *const1;
                {
                    auto Int_class = ljf_get_object_from_environment(env, "Int");
                    auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
                    auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
                    const1 = ljf_new_object_with_native_data(1);
                    ljf_push_object_to_array(tmp, const1);
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "self", const1);
                    ljf_call_function(Int_init, Int_class_env, args);
                }
                // x - 1
                LJFObject *v2;
                {
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "a", v);
                    ljf_set_object_to_table(args, "b", const1);
                    v2 = ljf_call_function(opSub, class_env, args);
                    ljf_push_object_to_array(tmp, v2);
                }

                auto x = ljf_get_object_from_environment(env, "x");
                auto y = v2;
                auto z = ljf_get_object_from_environment(env, "z");
                auto args = ljf_new_object();
                ljf_push_object_to_array(tmp, args);
                ljf_set_object_to_table(args, "y", y);
                ljf_set_object_to_table(args, "z", z);
                ljf_set_object_to_table(args, "x", x);
                auto tarai_ljf_obj = ljf_get_object_from_environment(env, "tarai_ljf");
                auto tarai_ljf_env = ljf_get_object_from_table(tarai_ljf_obj, "env");
                auto tarai_ljf = ljf_get_function_id_from_function_table(tarai_ljf_obj, "call");
                auto result = ljf_call_function(tarai_ljf, tarai_ljf_env, args);
                ljf_push_object_to_array(tmp, result);
                return result;
            }();
        // tarai(z - 1, x, y)
        auto result3 =
            [&]() {
                // y - 1
                auto v = ljf_get_object_from_environment(env, "z");
                auto v_class = ljf_get_object_from_table(v, "class");
                auto class_env = ljf_get_object_from_hidden_table(v_class, "env");
                auto opSub = ljf_get_function_id_from_function_table(v_class, "-");

                // 1
                LJFObject *const1;
                {
                    auto Int_class = ljf_get_object_from_environment(env, "Int");
                    auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
                    auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
                    const1 = ljf_new_object_with_native_data(1);
                    ljf_push_object_to_array(tmp, const1);
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "self", const1);
                    ljf_call_function(Int_init, Int_class_env, args);
                }
                // x - 1
                LJFObject *v2;
                {
                    auto args = ljf_new_object();
                    ljf_push_object_to_array(tmp, args);
                    ljf_set_object_to_table(args, "a", v);
                    ljf_set_object_to_table(args, "b", const1);
                    v2 = ljf_call_function(opSub, class_env, args);
                    ljf_push_object_to_array(tmp, v2);
                }

                auto x = ljf_get_object_from_environment(env, "x");
                auto y = ljf_get_object_from_environment(env, "y");
                auto z = v2;
                auto args = ljf_new_object();
                ljf_push_object_to_array(tmp, args);
                ljf_set_object_to_table(args, "y", y);
                ljf_set_object_to_table(args, "z", z);
                ljf_set_object_to_table(args, "x", x);
                auto tarai_ljf_obj = ljf_get_object_from_environment(env, "tarai_ljf");
                auto tarai_ljf_env = ljf_get_object_from_table(tarai_ljf_obj, "env");
                auto tarai_ljf = ljf_get_function_id_from_function_table(tarai_ljf_obj, "call");
                auto result = ljf_call_function(tarai_ljf, tarai_ljf_env, args);
                ljf_push_object_to_array(tmp, result);
                return result;
            }();
        auto tarai_ljf_obj = ljf_get_object_from_environment(env, "tarai_ljf");
        auto tarai_ljf_env = ljf_get_object_from_table(tarai_ljf_obj, "env");
        auto tarai_ljf = ljf_get_function_id_from_function_table(tarai_ljf_obj, "call");
        auto args = ljf_new_object();
        ljf_push_object_to_array(tmp, args);
        ljf_set_object_to_table(args, "x", result1);
        ljf_set_object_to_table(args, "y", result2);
        ljf_set_object_to_table(args, "z", result3);
        return ljf_call_function(tarai_ljf, tarai_ljf_env, args);
    }
}

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{
    initIntClass(env, module_func_table);

    auto tarai_ljf_id = ljf_get_function_id_from_function_table(module_func_table, "tarai_ljf");
    auto tarai_ljf_obj = ljf_new_object();
    ljf_push_object_to_array(env, tarai_ljf_obj);
    ljf_set_function_id_to_function_table(tarai_ljf_obj, "call", tarai_ljf_id);
    ljf_set_object_to_table(tarai_ljf_obj, "env", env);
    ljf_set_object_to_environment(env, "tarai_ljf", tarai_ljf_obj);
    int64_t x = 1;
    int64_t y = 0;
    int64_t z = 0;

    {
        std::cout << "-- tarai --" << std::endl;
        mygperf::ProfilerStart("tmp/tarai.prof");
        auto start = std::chrono::high_resolution_clock::now();
        auto r = tarai(x, y, z);
        auto end = std::chrono::high_resolution_clock::now();
        mygperf::ProfilerStop();
        auto elapsed = end - start;
        std::cout << "tarai(" << x << ", " << y << ", " << z << ") = " << r << std::endl;
        std::cout << "elapsed ms (native) " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    {
        std::cout << "-- tarai_ljf --" << std::endl;
        auto args = ljf_new_object();
        ljf_push_object_to_array(env, args);
        ljf_set_object_to_table(args, "x", newInt(env, env, x));
        ljf_set_object_to_table(args, "y", newInt(env, env, y));
        ljf_set_object_to_table(args, "z", newInt(env, env, z));
        mygperf::ProfilerStart("tmp/tarai-ljf.prof");
        auto start = std::chrono::high_resolution_clock::now();
        auto tarai_ljf_obj = ljf_get_object_from_environment(env, "tarai_ljf");
        auto tarai_ljf_env = ljf_get_object_from_table(tarai_ljf_obj, "env");
        auto tarai_ljf = ljf_get_function_id_from_function_table(tarai_ljf_obj, "call");
        auto r = ljf_call_function(tarai_ljf, tarai_ljf_env, args);
        ljf_push_object_to_array(env, r);
        auto end = std::chrono::high_resolution_clock::now();
        mygperf::ProfilerStop();
        auto elapsed = end - start;
        std::cout << "tarai(" << x << ", " << y << ", " << z << ") = " << r << std::endl;
        std::cout << "elapsed ms (native) " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    return env;
}

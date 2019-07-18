#include <iostream>
#include <chrono>

#include <ljf/runtime.hpp>

LJFObject* const true_ = ljf_new_object_with_native_data(1);
LJFObject* const false_ = ljf_new_object_with_native_data(0);

extern "C" {
LJFObject* intOpEq(LJFObject* env, LJFObject* tmp) {

    auto a = ljf_get_object_from_environment(env, "a");
    auto b = ljf_get_object_from_environment(env, "b");

    auto a_val = ljf_get_native_data(a);
    auto b_val = ljf_get_native_data(b);

    return a_val == b_val ? true_ : false_;
}

LJFObject* intOpLt(LJFObject* env, LJFObject* tmp) {

    auto a = ljf_get_object_from_environment(env, "a");
    auto b = ljf_get_object_from_environment(env, "b");

    auto a_val = ljf_get_native_data(a);
    auto b_val = ljf_get_native_data(b);

    std::cout << "a=" << a_val << " b=" << b_val << "\n";
    std::cout << "bool=" << (a_val < b_val) << "\n";
    return a_val < b_val ? true_ : false_;
}

LJFObject* intOpAdd(LJFObject* env, LJFObject* tmp) {

    auto a = ljf_get_object_from_environment(env, "a");
    auto b = ljf_get_object_from_environment(env, "b");

    auto a_val = ljf_get_native_data(a);
    auto b_val = ljf_get_native_data(b);

    auto c_val = a_val + b_val;
    auto c = ljf_new_object_with_native_data(c_val);

    auto Int = ljf_get_object_from_environment(env, "Int");
    assert(Int);
    auto tmp_arg = ljf_new_object();
    ljf_set_object_to_table(tmp_arg, "self", c);
    ljf_call_function(ljf_get_function_id_from_function_table(Int, "init"),
        ljf_get_object_from_hidden_table(Int, "init.env"),
        tmp_arg);
    return c;
}

LJFObject* intInit(LJFObject* env, LJFObject* tmp) {


    auto self = ljf_get_object_from_environment(env, "self");
    auto Int = ljf_get_object_from_environment(env, "Int");
    assert(Int);
    ljf_set_object_to_table(self, "class", Int);

    return ljf_undefined;
}

LJFObject* fibo_loop_ljf(LJFObject* env) {
    // TODO: temporary storage

    auto n = ljf_get_object_from_environment(env, "n");

    auto Int = ljf_get_object_from_environment(env, "Int");
    auto intInit = ljf_get_function_id_from_function_table(Int, "init");
    // auto intInitEnv = ljf_get_object_from_hidden_table(Int, "init.env");

    {
        auto tmp_arg = ljf_new_object();
        ljf_set_object_to_table(tmp_arg, "self", n);
        ljf_call_function(intInit, env, tmp_arg);
    }

    // long f_n1 = 1;
    auto f_n1 = ljf_new_object_with_native_data(1);
    auto tmp_arg00 = ljf_new_object();
    ljf_set_object_to_table(tmp_arg00, "self", f_n1);
    ljf_call_function(intInit, env, tmp_arg00);

    // long f_n0 = 0;
    auto f_n0 = ljf_new_object_with_native_data(0);
    auto tmp_arg01 = ljf_new_object();
    ljf_set_object_to_table(tmp_arg01, "self", f_n0);
    ljf_call_function(intInit, env, tmp_arg01);

    const auto const0 = ljf_new_object_with_native_data(0);
    {
        auto tmp_arg = ljf_new_object();
        ljf_set_object_to_table(tmp_arg, "self", const0);
        ljf_call_function(intInit, env, tmp_arg);
    }
    // if (n == 0) {
    //     return f_n0;
    // }
    const auto tmp_arg0 = ljf_new_object();
    ljf_set_object_to_table(tmp_arg0, "a", n);
    ljf_set_object_to_table(tmp_arg0, "b", const0);
    auto n_class = ljf_get_object_from_table(n, "class");
    const auto opEq = ljf_get_function_id_from_function_table(n_class, "==");
    if (ljf_call_function(opEq, ljf_undefined, tmp_arg0) == true_) {
        return f_n0;
    }

    // if (n == 1) {
    //     return f_n1;
    // }
    const auto const1 = ljf_new_object_with_native_data(1);
    {
        auto tmp_arg = ljf_new_object();
        ljf_set_object_to_table(tmp_arg, "self", const1);
        ljf_call_function(intInit, env, tmp_arg);
    }
    const auto tmp_arg1 = ljf_new_object();
    ljf_set_object_to_table(tmp_arg1, "a", n);
    ljf_set_object_to_table(tmp_arg1, "b", const0);
    if (ljf_call_function(opEq, ljf_undefined, tmp_arg1) == true_) {
        return f_n1;
    }

    // int k = 1;
    // long f_n2 = 0;
    auto k = const1;
    auto f_n2 = const0;

    while (true)
    {
        // while (k < n)
        auto k_class = ljf_get_object_from_table(k, "class");
        const auto opLt = ljf_get_function_id_from_function_table(k_class, "<");
        const auto tmp_arg2 = ljf_new_object();
        ljf_set_object_to_table(tmp_arg2, "a", k);
        ljf_set_object_to_table(tmp_arg2, "b", n);
        if (ljf_call_function(opLt, ljf_undefined, tmp_arg2) == false_) {
           break;
        }

        // f_n2 = f_n0 + f_n1;
        auto f_n0_class = ljf_get_object_from_table(f_n0, "class");
        const auto opAdd = ljf_get_function_id_from_function_table(f_n0_class, "+");
        const auto tmp_arg3 = ljf_new_object();
        ljf_set_object_to_table(tmp_arg3, "a", f_n0);
        ljf_set_object_to_table(tmp_arg3, "b", f_n1);
        f_n2 = ljf_call_function(opAdd, env, tmp_arg3);
        
        f_n0 = f_n1;
        f_n1 = f_n2;

        // k++;
        const auto opAdd2 = ljf_get_function_id_from_function_table(k_class, "+");
        const auto tmp_arg4 = ljf_new_object();
        ljf_set_object_to_table(tmp_arg4, "a", k);
        ljf_set_object_to_table(tmp_arg4, "b", const1);
        k = ljf_call_function(opAdd2, env, tmp_arg4);

        std::cout << "k = " << ljf_get_native_data(k) << "\n";
        std::cout << "f_n2 = " << ljf_get_native_data(f_n2) << "\n";
    }
    return f_n2;
}

long fibo_loop(const int n) {

    long f_n1 = 1;
    long f_n0 = 0;
    if (n == 0) {
        return f_n0;
    }
    if (n == 1) {
        return f_n1;
    }

    int k = 1;
    long f_n2 = 0;
    while (k < n)
    {
        f_n2 = f_n0 + f_n1;
        f_n0 = f_n1;
        f_n1 = f_n2;
        k++;
    }
    return f_n2;

}

} // extern "C"

extern "C" LJFObject* module_main(LJFObject* env, LJFObject* module_func_table) {

    try {


        auto Int = ljf_new_object();
        ljf_set_object_to_environment(env, "Int", Int);

        auto intInitId = ljf_get_function_id_from_function_table(module_func_table, "intInit");
        ljf_set_function_id_to_function_table(Int, "init", intInitId);
        ljf_set_object_to_hidden_table(Int, "init.env", env);

        auto intOpEqId = ljf_get_function_id_from_function_table(module_func_table, "intOpEq");
        ljf_set_function_id_to_function_table(Int, "==", intOpEqId);

        auto intOpLtId = ljf_get_function_id_from_function_table(module_func_table, "intOpLt");
        ljf_set_function_id_to_function_table(Int, "<", intOpLtId);

        auto intOpAddId = ljf_get_function_id_from_function_table(module_func_table, "intOpAdd");
        ljf_set_function_id_to_function_table(Int, "+", intOpAddId);

        auto start = std::chrono::system_clock::now();
        auto r = fibo_loop_ljf(env);
        auto end = std::chrono::system_clock::now();
        auto elapsed = end - start;
        std::cout << r << std::endl;
        std::cout << ljf_get_native_data(r) << std::endl;
        std::cout << "elapsed " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
        return r;
        // module_main(Object::create(nullptr));
    } catch(const std::exception& e) {
        std::cerr << e.what() << "\n";
        return ljf_undefined;
    } catch(...) {
        std::cerr << "unknown exception" << "\n";
        throw;
    }
    return ljf_undefined;
    
}

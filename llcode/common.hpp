#include <stdint.h>
#include <iostream>

#include <gperftools/profiler.h>

#include <ljf/runtime.hpp>

#include "../runtime-internal.hpp"

namespace mygperf
{
#ifdef FIBO_PROF
constexpr bool fibo_prof = true;
#else
constexpr bool fibo_prof = false;
#endif

void ProfilerStart(const char *arg)
{
    if (fibo_prof)
    {
        ::ProfilerStart(arg);
    }
}

void ProfilerStop()
{
    if (fibo_prof)
    {
        ::ProfilerStop();
    }
}
} // namespace mygperf

namespace holders
{
ljf::ObjectHolder const true_ = ljf_new_object_with_native_data(1);
ljf::ObjectHolder const false_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const break_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const return_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const continue_ = ljf_new_object_with_native_data(0);
} // namespace holders

namespace
{
const auto true_ = holders::true_.get();
const auto false_ = holders::false_.get();
const auto break_ = holders::break_.get();
const auto return_ = holders::return_.get();
const auto continue_ = holders::continue_.get();
} // namespace

extern "C"
{
    LJFObject *intOpEq(LJFObject *env, LJFObject *tmp)
    {

        auto a = ljf_get_object_from_environment(env, "a");
        auto b = ljf_get_object_from_environment(env, "b");

        auto a_val = ljf_get_native_data(a);
        auto b_val = ljf_get_native_data(b);

        return a_val == b_val ? true_ : false_;
    }

    LJFObject *intOpLt(LJFObject *env, LJFObject *tmp)
    {

        auto a = ljf_get_object_from_environment(env, "a");
        auto b = ljf_get_object_from_environment(env, "b");

        auto a_val = ljf_get_native_data(a);
        auto b_val = ljf_get_native_data(b);

        // std::cout << "a=" << a_val << " b=" << b_val << "\n";
        // std::cout << "bool=" << (a_val < b_val) << "\n";
        return a_val < b_val ? true_ : false_;
    }
    LJFObject *intOpLe(LJFObject *env, LJFObject *tmp)
    {

        auto a = ljf_get_object_from_environment(env, "a");
        auto b = ljf_get_object_from_environment(env, "b");

        auto a_val = ljf_get_native_data(a);
        auto b_val = ljf_get_native_data(b);

        // std::cout << "a=" << a_val << " b=" << b_val << "\n";
        // std::cout << "bool=" << (a_val < b_val) << "\n";
        return a_val <= b_val ? true_ : false_;
    }

    LJFObject *intOpAdd(LJFObject *env, LJFObject *tmp)
    {

        auto a = ljf_get_object_from_environment(env, "a");
        auto b = ljf_get_object_from_environment(env, "b");

        auto a_val = ljf_get_native_data(a);
        auto b_val = ljf_get_native_data(b);

        auto c_val = a_val + b_val;
        auto c = ljf_new_object_with_native_data(c_val);
        ljf_push_object_to_array(tmp, c);

        // std::cout << "intOpAdd: " << a_val << " + " << b_val << " = " << c_val << std::endl;
        // std::cout << "intOpAdd: ljf_get_native_data(c): " << ljf_get_native_data(c) << std::endl;
        assert(ljf_get_native_data(c) == c_val);

        auto Int = ljf_get_object_from_environment(env, "Int");
        assert(Int);
        auto tmp_arg = ljf_new_object();
        ljf_push_object_to_array(tmp, tmp_arg);

        ljf_set_object_to_table(tmp_arg, "self", c);
        ljf_call_function(ljf_get_function_id_from_function_table(Int, "init"),
                          ljf_get_object_from_hidden_table(Int, "init.env"),
                          tmp_arg);
        return c;
    }

    LJFObject *intOpSub(LJFObject *env, LJFObject *tmp)
    {

        auto a = ljf_get_object_from_environment(env, "a");
        auto b = ljf_get_object_from_environment(env, "b");

        auto a_val = ljf_get_native_data(a);
        auto b_val = ljf_get_native_data(b);

        auto c_val = a_val - b_val;
        auto c = ljf_new_object_with_native_data(c_val);
        ljf_push_object_to_array(tmp, c);

        // std::cout << "intOpAdd: " << a_val << " + " << b_val << " = " << c_val << std::endl;
        // std::cout << "intOpAdd: ljf_get_native_data(c): " << ljf_get_native_data(c) << std::endl;
        assert(ljf_get_native_data(c) == c_val);

        auto Int = ljf_get_object_from_environment(env, "Int");
        assert(Int);
        auto tmp_arg = ljf_new_object();
        ljf_push_object_to_array(tmp, tmp_arg);

        ljf_set_object_to_table(tmp_arg, "self", c);
        ljf_call_function(ljf_get_function_id_from_function_table(Int, "init"),
                          ljf_get_object_from_hidden_table(Int, "init.env"),
                          tmp_arg);
        return c;
    }

    LJFObject *intInit(LJFObject *env, LJFObject *tmp)
    {

        auto self = ljf_get_object_from_environment(env, "self");
        auto Int = ljf_get_object_from_environment(env, "Int");
        assert(Int);
        ljf_set_object_to_table(self, "class", Int);

        return ljf_undefined;
    }
}

extern "C" inline void initIntClass(LJFObject *env, LJFObject *module_func_table)
{
    auto Int = ljf_new_object();
    ljf_set_object_to_environment(env, "Int", Int);

    auto intInitId = ljf_get_function_id_from_function_table(module_func_table, "intInit");
    ljf_set_function_id_to_function_table(Int, "init", intInitId);
    ljf_set_object_to_hidden_table(Int, "init.env", env);
    ljf_set_object_to_hidden_table(Int, "env", env);

    auto intOpEqId = ljf_get_function_id_from_function_table(module_func_table, "intOpEq");
    ljf_set_function_id_to_function_table(Int, "==", intOpEqId);

    auto intOpLtId = ljf_get_function_id_from_function_table(module_func_table, "intOpLt");
    ljf_set_function_id_to_function_table(Int, "<", intOpLtId);

    auto intOpLeId = ljf_get_function_id_from_function_table(module_func_table, "intOpLe");
    ljf_set_function_id_to_function_table(Int, "<=", intOpLeId);

    auto intOpAddId = ljf_get_function_id_from_function_table(module_func_table, "intOpAdd");
    ljf_set_function_id_to_function_table(Int, "+", intOpAddId);

    auto intOpSubId = ljf_get_function_id_from_function_table(module_func_table, "intOpSub");
    ljf_set_function_id_to_function_table(Int, "-", intOpSubId);
}

inline LJFObject *newInt(LJFObject *env, LJFObject *tmp, uint64_t i)
{
    auto Int_class = ljf_get_object_from_environment(env, "Int");
    auto Int_class_env = ljf_get_object_from_hidden_table(Int_class, "env");
    auto Int_init = ljf_get_function_id_from_function_table(Int_class, "init");
    auto r = ljf_new_object_with_native_data(i);
    ljf_push_object_to_array(tmp, r);
    auto args = ljf_new_object();
    ljf_push_object_to_array(tmp, args);
    ljf_set_object_to_table(args, "self", r);
    ljf_call_function(Int_init, Int_class_env, args);
    return r;
}

inline void add_func_obj_to_env(LJFObject *env, LJFObject *module_func_table, const char *func_name)
{
    auto func_id = ljf_get_function_id_from_function_table(module_func_table, func_name);
    auto func_obj = ljf_new_object();
    ljf_push_object_to_array(env, func_obj);
    ljf_set_function_id_to_function_table(func_obj, "call", func_id);
    ljf_set_object_to_hidden_table(func_obj, "env", env);
    ljf_set_object_to_environment(env, func_name, func_obj);
}

inline void add_func_id_to_obj(LJFObject *obj, const char *func_name, LJFObject *module_func_table, const char *module_func_name)
{
    auto func_id = ljf_get_function_id_from_function_table(module_func_table, module_func_name);
    ljf_set_function_id_to_function_table(obj, func_name, func_id);
}

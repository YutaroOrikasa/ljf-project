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

    void ProfilerStart(const char* arg) {
        if (fibo_prof)
        {
            ::ProfilerStart(arg);
        }
    }

    void ProfilerStop() {
        if (fibo_prof)
        {
            ::ProfilerStop();
        }
    }
} // namespace mygperf


namespace holders {
ljf::ObjectHolder const true_ = ljf_new_object_with_native_data(1);
ljf::ObjectHolder const false_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const break_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const return_ = ljf_new_object_with_native_data(0);
ljf::ObjectHolder const continue_ = ljf_new_object_with_native_data(0);
}

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

    auto intOpEqId = ljf_get_function_id_from_function_table(module_func_table, "intOpEq");
    ljf_set_function_id_to_function_table(Int, "==", intOpEqId);

    auto intOpLtId = ljf_get_function_id_from_function_table(module_func_table, "intOpLt");
    ljf_set_function_id_to_function_table(Int, "<", intOpLtId);

    auto intOpAddId = ljf_get_function_id_from_function_table(module_func_table, "intOpAdd");
    ljf_set_function_id_to_function_table(Int, "+", intOpAddId);
}

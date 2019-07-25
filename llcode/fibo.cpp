#include <iostream>
#include <chrono>
#include <cstdint>
#include <vector>
#include <limits.h>
#include <iomanip>
#include <memory>

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
const auto true_ = holders::true_.get();
const auto false_ = holders::false_.get();
const auto break_ = holders::break_.get();
const auto return_ = holders::return_.get();
const auto continue_ = holders::continue_.get();

class BigInt
{
private:
    bool is_neg_;
    uint32_t value_;
    std::vector<uint32_t> big_values_;

public:
    explicit BigInt(int64_t value) : is_neg_(value < 0)
    {
        if (INT32_MIN <= value && value <= INT32_MAX)
        {
            value_ = value;
        }
        else
        {
            uint32_t low = value;
            uint32_t high = value >> 32;
            value_ = low;
            big_values_.push_back(high);
        }
        // std::cout << "new BigInt"
        //           << "\n";
        // dump(std::cout);
    }

    BigInt *add(const BigInt *rhs) const
    {
        int64_t value = this->small() + rhs->small();
        // std::cout << "*** value: " << value << " = " << this->small() << " + " << rhs->small() << "\n";

        if (this->is_small() && rhs->is_small())
        {
            return new BigInt(value);
        }

        auto ret = new BigInt(0);

        auto bigger = this->big_values_.size() > rhs->big_values_.size() ? this : rhs;
        ret->big_values_.reserve(bigger->big_values_.size() + 1);
        const uint32_t small = value;
        uint32_t carry = value >> 32;

        ret->value_ = small;

        auto common_size = std::min(this->big_values_.size(), rhs->big_values_.size());

        for (size_t i = 0; i < common_size; i++)
        {
            // void DEBUG_loop_start();
            // DEBUG_loop_start();
            int64_t left = this->big_values_.at(i);
            int64_t right = rhs->big_values_.at(i);
            int64_t carry64 = carry;

            int64_t result = carry64 + left + right;

            const int32_t small = result;
            carry = result >> 32;

            // std::cout << "--------\n";
            // std::cout << "left: " << left << "\n";
            // std::cout << "right: " << right << "\n";
            // std::cout << "carry64: " << carry64 << "\n";
            // std::cout << "\n";

            // std::cout << "result: " << result << "\n";
            // std::cout << "small: " << small << "\n";
            // std::cout << "carry: " << carry << "\n";
            // std::cout << "\n";

            ret->big_values_.push_back(small);

            // void DEBUG_loop_end();
            // DEBUG_loop_end();
        }

        for (size_t i = common_size; i < bigger->big_values_.size(); i++)
        {
            int64_t b = bigger->big_values_.at(i);
            int64_t carry64 = carry;
            int64_t result = carry64 + b;
            const uint32_t small = result;
            carry = result >> 32;

            // std::cout << "--------\n";
            // std::cout << "b: " << b << "\n";
            // std::cout << "carry64: " << carry64 << "\n";
            // std::cout << "\n";

            // std::cout << "result: " << result << "\n";
            // std::cout << "small: " << small << "\n";
            // std::cout << "carry: " << carry << "\n";
            // std::cout << "\n";

            ret->big_values_.push_back(small);
        }

        if (carry != 0)
        {
            ret->big_values_.push_back(carry);
        }

        ret->is_neg_ = (*--ret->big_values_.end() < 0);
        return ret;
    }

    bool equals(const BigInt *other) const
    {

        if (this->value_ != other->value_)
        {
            return false;
        }

        auto common_size = std::min(this->big_values_.size(), other->big_values_.size());

        for (size_t i = 0; i < common_size; i++)
        {
            if (this->big_values_.at(i) != other->big_values_.at(i))
            {
                return false;
            }
        }

        auto bigger = this->big_values_.size() > other->big_values_.size() ? this : other;

        for (size_t i = common_size; i < bigger->big_values_.size(); i++)
        {
            int64_t b = bigger->big_values_.at(i);
            if (b != 0)
            {
                return false;
            }
        }

        return true;
    }

    bool equals(int32_t value) const
    {
        if (!is_small())
        {
            return false;
        }

        return value_ == value;
    }

    template <typename Out>
    Out &dump(Out &out) const
    {
        out << "is_neg_: " << is_neg_ << "\n";
        out << "value_: " << value_ << "\n";
        out << "vector: [";
        for (auto &&v : big_values_)
        {
            out << v << ", ";
        }
        out << "]\n";
        return out;
    }

    size_t big_size() const 
    {
        return big_values_.size();
    }

    template <typename Out>
    friend auto &operator<<(Out &out, const BigInt &bi);

private:
    bool is_small() const
    {
        return big_values_.size() == 0;
    }

    int64_t small() const
    {
        if (is_neg_)
        {
            return static_cast<int32_t>(value_);
        }
        else
        {
            return static_cast<uint32_t>(value_);
        }
    }
};

template <typename Out>
auto &operator<<(Out &out, const BigInt &bi)
{
    std::string str;
    out << std::hex;
    out << "0x";
    for (int i = bi.big_values_.size() - 1; i >= 0; i--)
    {
        out << (uint32_t)bi.big_values_.at(i);
    }
    out << (uint32_t)bi.value_;
    out << std::resetiosflags(std::ios_base::basefield);
    return out;
}

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

    LJFObject *fibo_loop_ljf_loop_closure_fn(LJFObject *env, LJFObject *tmp)
    {
        // std::cout << "fibo_loop_ljf_loop_closure_fn: k: " << ljf_get_object_from_environment(env, "k") << std::endl;
        const auto const1 = ljf_new_object_with_native_data(1);
        ljf_push_object_to_array(tmp, const1);
        // while (k < n)
        {
            auto k = ljf_get_object_from_environment(env, "k");
            auto k_class = ljf_get_object_from_table(k, "class");
            const auto opLt = ljf_get_function_id_from_function_table(k_class, "<");
            const auto tmp_arg2 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg2);
            ljf_set_object_to_table(tmp_arg2, "a", k);
            ljf_set_object_to_table(tmp_arg2, "b", ljf_get_object_from_environment(env, "n"));
            if (ljf_call_function(opLt, ljf_undefined, tmp_arg2) == false_)
            {
                // std::cout << "break_" << std::endl;
                return break_;
            }
        }

        // f_n2 = f_n0 + f_n1;
        {
            auto f_n0 = ljf_get_object_from_environment(env, "f_n0");
            auto f_n0_class = ljf_get_object_from_table(f_n0, "class");
            const auto opAdd = ljf_get_function_id_from_function_table(f_n0_class, "+");
            const auto tmp_arg3 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg3);
            ljf_set_object_to_table(tmp_arg3, "a", f_n0);
            ljf_set_object_to_table(tmp_arg3, "b", ljf_get_object_from_environment(env, "f_n1"));
            auto f_n2 = ljf_call_function(opAdd, env, tmp_arg3);
            // std::cout << "DEBUG10" << std::endl;

            ljf_set_object_to_environment(env, "f_n2", f_n2);
            // std::cout << "DEBUG11" << std::endl;
        }

        // f_n0 = f_n1;
        // f_n1 = f_n2;
        {
            ljf_set_object_to_environment(env, "f_n0", ljf_get_object_from_environment(env, "f_n1"));
            ljf_set_object_to_environment(env, "f_n1", ljf_get_object_from_environment(env, "f_n2"));
        }

        // k++;
        {
            auto k = ljf_get_object_from_environment(env, "k");
            auto k_class = ljf_get_object_from_table(k, "class");
            const auto opAdd2 = ljf_get_function_id_from_function_table(k_class, "+");
            const auto tmp_arg4 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg4);
            ljf_set_object_to_table(tmp_arg4, "a", k);
            ljf_set_object_to_table(tmp_arg4, "b", const1);
            auto k2 = ljf_call_function(opAdd2, env, tmp_arg4);
            ljf_set_object_to_environment(env, "k", k2);
        }

        // std::cout << "k = " << ljf_get_native_data(ljf_get_object_from_environment(env, "k")) << "\n";
        // std::cout << "f_n2 = " << ljf_get_native_data(ljf_get_object_from_environment(env, "f_n2")) << "\n";
        // std::cout << "fibo_loop_ljf_loop_closure_fn: k2: " << ljf_get_object_from_environment(env, "k") << std::endl;
        // std::cout << "continue_ = " << continue_ << std::endl;
        return continue_;
    }

    LJFObject *fibo_loop_ljf(LJFObject *env, LJFObject *tmp)
    {

        auto Int = ljf_get_object_from_environment(env, "Int");
        // std::cout << "DEBUG" << std::endl;
        auto intInit = ljf_get_function_id_from_function_table(Int, "init");
        // auto intInitEnv = ljf_get_object_from_hidden_table(Int, "init.env");

        // std::cout << "DEBUG2" << std::endl;
        {
            auto tmp_arg = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg);
            ljf_set_object_to_table(tmp_arg, "self", ljf_get_object_from_environment(env, "n"));
            ljf_call_function(intInit, env, tmp_arg);
        }
        // std::cout << "DEBUG3" << std::endl;

        // long f_n1 = 1;
        {
            auto f_n1 = ljf_new_object_with_native_data(1);
            ljf_push_object_to_array(tmp, f_n1);
            auto tmp_arg00 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg00);
            ljf_set_object_to_table(tmp_arg00, "self", f_n1);
            ljf_call_function(intInit, env, tmp_arg00);
            // std::cout << "DEBUG4" << std::endl;
            ljf_set_object_to_environment(env, "f_n1", f_n1);
        }

        // long f_n0 = 0;
        {
            auto f_n0 = ljf_new_object_with_native_data(0);
            ljf_push_object_to_array(tmp, f_n0);
            auto tmp_arg01 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg01);
            ljf_set_object_to_table(tmp_arg01, "self", f_n0);
            ljf_call_function(intInit, env, tmp_arg01);
            ljf_set_object_to_environment(env, "f_n0", f_n0);
        }

        const auto const0 = ljf_new_object_with_native_data(0);
        ljf_push_object_to_array(tmp, const0);
        {
            auto tmp_arg = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg);
            ljf_set_object_to_table(tmp_arg, "self", const0);
            ljf_call_function(intInit, env, tmp_arg);
        }
        // if (n == 0) {
        //     return f_n0;
        // }
        {
            const auto tmp_arg0 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg0);
            
            auto n = ljf_get_object_from_environment(env, "n");
            ljf_set_object_to_table(tmp_arg0, "a", n);
            ljf_set_object_to_table(tmp_arg0, "b", const0);
            auto n_class = ljf_get_object_from_table(n, "class");
            const auto opEq = ljf_get_function_id_from_function_table(n_class, "==");
            if (ljf_call_function(opEq, ljf_undefined, tmp_arg0) == true_)
            {
                return ljf_get_object_from_environment(env, "f_n0");
            }
        }

        // if (n == 1) {
        //     return f_n1;
        // }
        const auto const1 = ljf_new_object_with_native_data(1);
        ljf_push_object_to_array(tmp, const1);
        {
            auto tmp_arg = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg);
            ljf_set_object_to_table(tmp_arg, "self", const1);
            ljf_call_function(intInit, env, tmp_arg);
        }
        {
            const auto tmp_arg1 = ljf_new_object();
            ljf_push_object_to_array(tmp, tmp_arg1);
            auto n = ljf_get_object_from_environment(env, "n");
            ljf_set_object_to_table(tmp_arg1, "a", n);
            ljf_set_object_to_table(tmp_arg1, "b", const1);
            auto n_class = ljf_get_object_from_table(n, "class");
            const auto opEq = ljf_get_function_id_from_function_table(n_class, "==");
            if (ljf_call_function(opEq, ljf_undefined, tmp_arg1) == true_)
            {
                return ljf_get_object_from_environment(env, "f_n1");
            }
        }

        // int k = 1;
        // long f_n2 = 0;
        ljf_set_object_to_environment(env, "k", const1);
        ljf_set_object_to_environment(env, "f_n2", const0);

        while (true)
        {
            // std::cout << "fibo_loop_ljf: k: " << ljf_get_object_from_environment(env, "k") << std::endl;
            auto fn_obj = ljf_get_object_from_environment(env, "fibo_loop_ljf_loop_closure_fn");
            auto fn_id = ljf_get_function_id_from_function_table(fn_obj, "call");
            auto controle = ljf_call_function(fn_id, env, nullptr);
            if (controle == break_)
            {
                // std::cout << "controle = " << controle << std::endl;
                // std::cout << "controle == break_" << std::endl;
                break;
            }
        }
        return ljf_get_object_from_environment(env, "f_n2");
    }

    LJFObject *fibo_loop_ljf_local_ptr(LJFObject *env)
    {
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
        if (ljf_call_function(opEq, ljf_undefined, tmp_arg0) == true_)
        {
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
        ljf_set_object_to_table(tmp_arg1, "b", const1);
        if (ljf_call_function(opEq, ljf_undefined, tmp_arg1) == true_)
        {
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
            if (ljf_call_function(opLt, ljf_undefined, tmp_arg2) == false_)
            {
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

            // std::cout << "k = " << ljf_get_native_data(k) << "\n";
            // std::cout << "f_n2 = " << ljf_get_native_data(f_n2) << "\n";
        }
        return f_n2;
    }

    uint64_t fibo_loop(uint64_t n)
    {

        uint64_t f_n1 = 1;
        uint64_t f_n0 = 0;
        if (n == 0)
        {
            return f_n0;
        }
        if (n == 1)
        {
            return f_n1;
        }

        uint64_t k = 1;
        uint64_t f_n2 = 0;
        while (k < n)
        {
            f_n2 = f_n0 + f_n1;
            f_n0 = f_n1;
            f_n1 = f_n2;
            k++;
        }
        return f_n2;
    }

    std::shared_ptr<BigInt> fibo_loop_big_int(uint64_t n)
    {
        auto f_n1 = std::make_shared<BigInt>(1);
        auto f_n0 = std::make_shared<BigInt>(0);

        if (n == 0)
        {
            return f_n0;
        }

        if (n == 1)
        {
            return f_n1;
        }
        uint64_t k = 1;
        auto f_n2 = std::make_shared<BigInt>(0);

        while (k < n)
        {
            // std::cout << "lhs=" << *f_n0 << " + rhs=" << *f_n1 << "\n";
            f_n2.reset(f_n0->add(f_n1.get()));
            // std::cout << "result=" << *f_n2 << "\n";
            // f_n2->dump(std::cout);
            f_n0 = f_n1;
            f_n1 = f_n2;
            k++;
            // std::cout << "k=" << k << ": " << *f_n2 << std::endl;
        }
        return f_n2;
    }

} // extern "C"

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{

    try
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

        auto fibo_loop_ljf_loop_closure_fn_id = ljf_get_function_id_from_function_table(module_func_table, "fibo_loop_ljf_loop_closure_fn");
        auto fibo_loop_ljf_loop_closure_fn_obj = ljf_new_object();
        ljf_set_object_to_environment(env, "fibo_loop_ljf_loop_closure_fn", fibo_loop_ljf_loop_closure_fn_obj);
        ljf_set_function_id_to_function_table(fibo_loop_ljf_loop_closure_fn_obj, "call", fibo_loop_ljf_loop_closure_fn_id);

        ljf_set_object_to_environment(env, "n", ljf_new_object_with_native_data(1 << 17)); // 1 << 21
        LJFObject *r;
        {
            auto n = ljf_get_native_data(ljf_get_object_from_environment(env, "n"));
            std::cout << "n " << n << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            auto r = fibo_loop(n);
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = end - start;
            std::cout << r << std::endl;
            std::cout << "elapsed ms (native) " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
        }
        {
            std::cout << "-- fibo_loop_big_int --" << std::endl;
            auto n = ljf_get_native_data(ljf_get_object_from_environment(env, "n"));
            std::cout << "n " << n << std::endl;
            mygperf::ProfilerStart("tmp/fibo-bigint.prof");
            auto start = std::chrono::high_resolution_clock::now();
            auto r = fibo_loop_big_int(n);
            auto end = std::chrono::high_resolution_clock::now();
            mygperf::ProfilerStop();
            auto elapsed = end - start;
            std::cout << *r << std::endl;
            std::cout << "big_size: " << r->big_size() << std::endl;
            std::cout << "mem size: " << r->big_size() * sizeof(int32_t) << std::endl;
            std::cout << "elapsed ms (native) " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
        }
        // {
        //     std::cout << "-- fibo_loop_ljf_local_ptr --" << std::endl;
        //     auto start = std::chrono::system_clock::now();
        //     auto r = fibo_loop_ljf_local_ptr(env);
        //     auto end = std::chrono::system_clock::now();
        //     auto elapsed = end - start;
        //     std::cout << r << std::endl;
        //     std::cout << ljf_get_native_data(r) << std::endl;
        //     std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
        // }
        {
            std::cout << "-- fibo_loop_ljf --" << std::endl;
            mygperf::ProfilerStart("tmp/fibo-ljf.prof");
            auto start = std::chrono::system_clock::now();
            auto fn = ljf_get_function_id_from_function_table(module_func_table, "fibo_loop_ljf");
            r = ljf_call_function(fn, env, ljf_new_object());
            auto end = std::chrono::system_clock::now();
            mygperf::ProfilerStop();
            auto elapsed = end - start;
            std::cout << r << std::endl;
            std::cout << ljf_get_native_data(r) << std::endl;
            std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
        }
        // for (size_t i = 0; i <= 100; i++)
        // {
        //     auto f = fibo_loop(i);
        //     ljf_set_object_to_environment(env, "n", ljf_new_object_with_native_data(i));
        //     auto fl = ljf_get_native_data(fibo_loop_ljf(env));
        //     if (f != fl) {
        //         std::cout << "n = " << i << ": (fibo: " << f << ") != (fibo-ljf: " << fl << ")" << std::endl;
        //         break;
        //     }
        // }

        return r;
        // module_main(Object::create(nullptr));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n";
        return ljf_undefined;
    }
    catch (...)
    {
        std::cerr << "unknown exception"
                  << "\n";
        throw;
    }
    return ljf_undefined;
}

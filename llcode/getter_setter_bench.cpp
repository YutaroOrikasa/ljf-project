#include "common.hpp"

class GS
{
    uint64_t value_ = 0;

public:
    explicit GS(uint64_t v) : value_(v) {}
    uint64_t get() const
    {
        return value_;
    }
    void set(uint64_t value)
    {
        value_ = value;
    }
};

uint64_t getter_setter_bench(uint64_t n)
{
    uint64_t k = 1;
    auto G = std::make_shared<GS>(0);
    auto S = std::make_shared<GS>(0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + G->get();
        S->set(r);
        auto tmp = G;
        G = S;
        S = tmp;
    }

    return S->get();
}

namespace gs_shared_int
{
class GS
{
    std::shared_ptr<uint64_t> value_ = {0};

public:
    explicit GS(uint64_t v) : GS(std::make_shared<uint64_t>(v)) {}
    explicit GS(std::shared_ptr<uint64_t> v) : value_(v) {}
    std::shared_ptr<uint64_t> get() const
    {
        return value_;
    }
    void set(std::shared_ptr<uint64_t> value)
    {
        value_ = value;
    }
};

uint64_t getter_setter_bench(void *, uint64_t n)
{
    uint64_t k = 1;
    auto G = std::make_shared<GS>(0);
    auto S = std::make_shared<GS>(0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + *G->get();
        S->set(std::make_shared<uint64_t>(r));
        auto tmp = G;
        G = S;
        S = tmp;
    }

    return *S->get();
}
} // namespace gs_shared_int

extern "C"
{
    LJFObject *GS_new(LJFObject *env, LJFObject *tmp)
    {
        auto cls = ljf_get_object_from_environment(env, "cls");
        auto self = ljf_new_object();
        ljf_set_object_to_table(self, "class", cls);
        add_func_id_to_obj(self, "get", cls, "get");
        add_func_id_to_obj(self, "set", cls, "set");
        return self;
    }

    LJFObject *GS_init(LJFObject *env, LJFObject *tmp)
    {
        auto v = ljf_get_object_from_environment(env, "v");
        assert(v);
        auto self = ljf_get_object_from_environment(env, "self");
        ljf_set_object_to_table(self, "_value", v);
        return ljf_undefined;
    }

    LJFObject *GS_get(LJFObject *env, LJFObject *tmp)
    {
        auto self = ljf_get_object_from_environment(env, "self");
        auto v = ljf_get_object_from_table(self, "_value");
        return v;
    }

    LJFObject *GS_set(LJFObject *env, LJFObject *tmp)
    {
        auto self = ljf_get_object_from_environment(env, "self");
        auto v = ljf_get_object_from_environment(env, "v");
        assert(v);
        ljf_set_object_to_table(self, "_value", v);
        return ljf_undefined;
    }
} // extern "C"

static ljf::ObjectHolder newGS(LJFObject *env, uint64_t v)
{
    auto GS = ljf_get_object_from_environment(env, "GS");
    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_set_object_to_table(args, "cls", GS);
    auto new_id = ljf_get_function_id_from_function_table(GS, "new");
    ljf::ObjectHolder self = ljf_call_function(new_id, env, args);

    {
        auto args = ljf_new_object();
        ljf::ObjectHolder args_h = args;
        ljf_set_object_to_table(args, "self", self.get());
        ljf_set_object_to_table(args, "v", ljf_new_object_with_native_data(v));
        auto init_id = ljf_get_function_id_from_function_table(GS, "init");
        ljf_call_function(init_id, env, args);
    }

    return self;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_set_object_to_table(args, "self", gs);
    auto fid = ljf_get_function_id_from_function_table(gs, "get");
    return ljf_call_function(fid, env, args);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_set_object_to_table(args, "self", gs);
    ljf::ObjectHolder v_h = ljf_new_object_with_native_data(v);
    ljf_set_object_to_table(args, "v", v_h.get());
    auto fid = ljf_get_function_id_from_function_table(gs, "set");
    ljf_call_function(fid, env, args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);
        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}

// ------------------- direct_call
LJFObject *direct_GS_get(LJFObject *args, LJFObject *tmp)
{
    auto self = ljf_get_object_from_table(args, "self");
    auto v = ljf_get_object_from_table(self, "_value");
    return v;
}

LJFObject *direct_GS_set(LJFObject *args, LJFObject *tmp)
{
    auto self = ljf_get_object_from_table(args, "self");
    auto v = ljf_get_object_from_table(args, "v");
    assert(v);
    ljf_set_object_to_table(self, "_value", v);
    return ljf_undefined;
}

static ljf::ObjectHolder direct_getGS(LJFObject *env, LJFObject *gs)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_set_object_to_table(args, "self", gs);
    return direct_GS_get(args, nullptr);
}

static void direct_setGS(LJFObject *env, LJFObject *gs, uint64_t v)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_set_object_to_table(args, "self", gs);
    ljf::ObjectHolder v_h = ljf_new_object_with_native_data(v);
    ljf_set_object_to_table(args, "v", v_h.get());
    direct_GS_set(args, nullptr);
}
uint64_t direct_getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(direct_getGS(env, G.get()).get());
        direct_setGS(env, S.get(), r);
        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(direct_getGS(env, S.get()).get());
}

// ------------------- direct_call_index_access
namespace direct_call_index_access
{

LJFObject *GS_get(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    auto self = ljf_internal_get_object_by_index(args, 0);
    // auto v_ = ljf_get_object_from_table(self, "_value");
    auto v = ljf_internal_get_object_by_index(self, 1);
    // assert(v == v_);
    return v;
}

LJFObject *GS_set(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    auto self = ljf_internal_get_object_by_index(args, 0);
    // auto v = ljf_get_object_from_table(args, "v");
    auto v = ljf_internal_get_object_by_index(args, 1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    ljf_internal_set_object_by_index(self, 1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_internal_resize_object_array_table_size(args, 1);
    // ljf_set_object_to_table(args, "self", gs);
    ljf_internal_set_object_by_index(args, 0, gs);
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{

    auto args = ljf_new_object();
    ljf::ObjectHolder args_h = args;
    ljf_internal_resize_object_array_table_size(args, 2);
    // ljf_set_object_to_table(args, "self", gs);
    ljf_internal_set_object_by_index(args, 0, gs);
    ljf::ObjectHolder v_h = ljf_new_object_with_native_data(v);
    // ljf_set_object_to_table(args, "v", v_h.get());
    ljf_internal_set_object_by_index(args, 1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access

#include "getter_setter_bench_inline.hpp"

template <typename F>
void bench_ljf(const char *name, LJFObject *env, size_t n, const F &bench_fn)
{
    std::cout << "-- " << name << " --" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto r = bench_fn(env, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end - start;
    std::cout << name << "(" << n << ") = " << r << std::endl;
    std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
}

extern "C" LJFObject *module_main(LJFObject *env, LJFObject *module_func_table)
{
    auto GS = ljf_new_object();
    ljf_set_object_to_environment(env, "GS", GS);
    add_func_id_to_obj(GS, "new", module_func_table, "GS_new");
    add_func_id_to_obj(GS, "init", module_func_table, "GS_init");
    add_func_id_to_obj(GS, "get", module_func_table, "GS_get");
    add_func_id_to_obj(GS, "set", module_func_table, "GS_set");
    // add_func_obj_to_env(env, module_func_table, "");

    // auto n = (1 << 23);
    auto n = (1 << 20);

    {
        std::cout << "-- getter_setter_bench --" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        auto r = getter_setter_bench(n);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << "getter_setter_bench(" << n << ") = " << r << std::endl;
        std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    }

    bench_ljf("gs_shared_int",
              env, n, gs_shared_int::getter_setter_bench);
    // {
    //     std::cout << "-- getter_setter_bench_ljf --" << std::endl;
    //     auto start = std::chrono::high_resolution_clock::now();
    //     auto r = getter_setter_bench_ljf(env, n);
    //     auto end = std::chrono::high_resolution_clock::now();
    //     auto elapsed = end - start;
    //     std::cout << "getter_setter_bench_ljf(" << n << ") = " << r << std::endl;
    //     std::cout << "elapsed ms " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << std::endl;
    // }
    std::cout << std::endl;

    bench_ljf("newGS_costs",
              env, n, newGS_costs::getter_setter_bench_ljf);

    std::cout << std::endl;

    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_primitive_optimization",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_primitive_optimization::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object_primitive_optimization",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object_primitive_optimization::getter_setter_bench_ljf);
    std::cout << std::endl;
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_efficient_ObjectHolder_swap",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_efficient_ObjectHolder_swap::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_neverObjectHolder",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_neverObjectHolder::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_ljf_object_layout",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_ljf_object_layout::getter_setter_bench_ljf);
    std::cout << std::endl;

    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_big_object",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_big_object::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_small_object",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_small_object::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_small_object",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_small_object::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock_refactor",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock_refactor::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_native_arg_passing",
              env, n, direct_call_index_access_inline_native_arg_passing::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_nolock",
              env, n, direct_call_index_access_inline_args_struct_stack_nolock::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_stack_lock",
              env, n, direct_call_index_access_inline_args_struct_stack_lock::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_heap_nolock",
              env, n, direct_call_index_access_inline_args_struct_heap_nolock::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_args_struct_heap_lock",
              env, n, direct_call_index_access_inline_args_struct_heap_lock::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_stackallocated_holding",
              env, n, direct_call_index_access_inline_stackallocated_holding::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline_stackallocated",
              env, n, direct_call_index_access_inline_stackallocated::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access_inline", env, n, direct_call_index_access_inline::getter_setter_bench_ljf);
    bench_ljf("direct_call_index_access::getter_setter_bench_ljf", env, n, direct_call_index_access::getter_setter_bench_ljf);
    bench_ljf("direct_getter_setter_bench_ljf", env, n, direct_getter_setter_bench_ljf);
    bench_ljf("getter_setter_bench_ljf", env, n, getter_setter_bench_ljf);
    std::cout << "-------------" << std::endl;

    return env;
}

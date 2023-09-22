#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/internal/ObjectHolder.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;
using namespace ljf::internal;

TEST(LJFSetGet, SetGetVisible) {
    ObjectWrapper obj = make_new_wrapped_object();
    ObjectWrapper elem = make_new_wrapped_object();
    obj.set("elem", elem);

    ASSERT_EQ(elem, obj.get("elem"));
    ASSERT_THROW(obj.get_hidden("elem"), std::out_of_range);
}

TEST(LJFSetGet, SetGetHidden) {
    ObjectWrapper obj = make_new_wrapped_object();
    ObjectWrapper elem = make_new_wrapped_object();
    obj.set_hidden("elem", elem);

    ASSERT_EQ(elem, obj.get_hidden("elem"));

    ASSERT_THROW(obj.get("elem"), std::out_of_range);
}

TEST(LJFEnvironment, GetOuterValue) {
    using namespace ljf::internal;
    auto ctx = make_temporary_context();
    ObjectHolder obj = make_new_held_object();
    ObjectHolder env0 = create_environment(ctx.get());
    ljf_environment_set(ctx.get(), env0, cast_to_ljf_handle("obj"),
                        obj.get_handle(*ctx), LJF_ATTR_VISIBLE);
    ObjectHolder env = create_callee_environment(env0, nullptr);
    ASSERT_EQ(obj,
              ctx->get_from_handle(ljf_environment_get(ctx.get(), env, cast_to_ljf_handle("obj"),
                                  LJF_ATTR_VISIBLE)));
}

TEST(LJFEnvironment, GetNotExistValue) {
    using namespace ljf::internal;
    auto ctx = make_temporary_context();
    ObjectHolder env0 = create_environment(ctx.get());
    ObjectHolder env = create_callee_environment(env0, nullptr);

    ASSERT_THROW(ljf_environment_get(ctx.get(), env,
                                     cast_to_ljf_handle("not_exist"),
                                     LJF_ATTR_VISIBLE),
                 std::out_of_range);
}

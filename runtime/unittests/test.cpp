#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/runtime.hpp"
#include "ljf/ObjectWrapper.hpp"

using namespace ljf;
using namespace ljf::internal;

TEST(LJFSetGet, SetGetVisible) {
    auto ctx = make_temporary_context();
    ObjectWrapper obj = ljf_new(ctx.get());
    ObjectWrapper elem = ljf_new(ctx.get());
    obj.set("elem", elem);

    ASSERT_EQ(elem, obj.get("elem"));
    ASSERT_THROW(obj.get_hidden("elem"), std::out_of_range);
}

TEST(LJFSetGet, SetGetHidden) {
    auto ctx = make_temporary_context();
    ObjectWrapper obj = ljf_new(ctx.get());
    ObjectWrapper elem = ljf_new(ctx.get());
    obj.set_hidden("elem", elem);

    ASSERT_EQ(elem, obj.get_hidden("elem"));

    ASSERT_THROW(obj.get("elem"), std::out_of_range);
}

TEST(LJFEnvironment, GetOuterValue) {
    using namespace ljf::internal;
    auto ctx = make_temporary_context();
    ObjectHolder obj = ljf_new(ctx.get());
    ObjectHolder env0 = create_environment(ctx.get());
    ljf_environment_set(ctx.get(), env0, cast_to_ljf_handle("obj"), obj,
                        LJFAttribute::VISIBLE);
    ObjectHolder env = create_callee_environment(env0, nullptr);
    ASSERT_EQ(obj,
              ljf_environment_get(ctx.get(), env, cast_to_ljf_handle("obj"),
                                  LJFAttribute::VISIBLE));
    ASSERT_THROW(ljf_environment_get(ctx.get(), env,
                                     cast_to_ljf_handle("not_exist"),
                                     LJFAttribute::VISIBLE),
                 std::out_of_range);
}

TEST(ObjectHolder, NotEqual) {
    auto ctx = make_temporary_context();
    ObjectHolder obj1 = ljf_new(ctx.get());
    ObjectHolder obj2 = ljf_new(ctx.get());

    ASSERT_EQ(obj1, obj1);
    ASSERT_NE(obj1, obj2);
}

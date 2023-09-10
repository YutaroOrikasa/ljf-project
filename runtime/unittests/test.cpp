#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;
using namespace ljf::internal;

TEST(LJFSetGet, SetGetVisible) {
    auto ctx = make_temporary_context();
    ObjectHolder obj = ljf_new(ctx.get());
    ObjectHolder elem = ljf_new(ctx.get());
    ljf_set(ctx.get(), obj.get(), cast_to_ljf_handle("elem"), elem.get(),
            LJFAttribute::VISIBLE);

    ASSERT_EQ(elem.get(), ljf_get(ctx.get(), obj.get(), cast_to_ljf_handle("elem"),
                                  LJFAttribute::VISIBLE));
    ASSERT_THROW(
        ljf_get(ctx.get(), obj.get(), cast_to_ljf_handle("elem"), LJFAttribute::HIDDEN),
        std::out_of_range);
}

TEST(LJFSetGet, SetGetHidden) {
    auto ctx = make_temporary_context();
    ObjectHolder obj = ljf_new(ctx.get());
    ObjectHolder elem = ljf_new(ctx.get());
    ljf_set(ctx.get(), obj.get(), cast_to_ljf_handle("elem"), elem.get(),
            LJFAttribute::HIDDEN);

    ASSERT_EQ(elem.get(),
              ljf_get(ctx.get(), obj.get(), cast_to_ljf_handle("elem"),
                      LJFAttribute::HIDDEN));
    ASSERT_THROW(ljf_get(ctx.get(), obj.get(), cast_to_ljf_handle("elem"),
                         LJFAttribute::VISIBLE),
                 std::out_of_range);
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

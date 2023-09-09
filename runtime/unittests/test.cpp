#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;

TEST(LJFSetGet, SetGetVisible) {
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set(obj.get(),cast_to_ljf_handle("elem"), elem.get(), LJFAttribute::VISIBLE);

    ASSERT_EQ(elem.get(), ljf_get(obj.get(),cast_to_ljf_handle("elem"), LJFAttribute::VISIBLE));
    ASSERT_THROW(ljf_get(obj.get(),cast_to_ljf_handle("elem"), LJFAttribute::HIDDEN), std::out_of_range);
}

TEST(LJFSetGet, SetGetHidden) {
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set(obj.get(),cast_to_ljf_handle("elem"), elem.get(), LJFAttribute::HIDDEN);

    ASSERT_EQ(elem.get(), ljf_get(obj.get(),cast_to_ljf_handle("elem"), LJFAttribute::HIDDEN));
    ASSERT_THROW(ljf_get(obj.get(),cast_to_ljf_handle("elem"), LJFAttribute::VISIBLE), std::out_of_range);
}

TEST(LJFEnvironment, GetOuterValue) {
    using namespace ljf::internal;
    ObjectHolder obj = ljf_new_object();
    ObjectHolder env0 = create_environment();
    ljf_environment_set(env0, cast_to_ljf_handle("obj"), obj, LJFAttribute::VISIBLE);
    ObjectHolder env = create_callee_environment(env0, nullptr);
    ASSERT_EQ(obj, ljf_environment_get(env, cast_to_ljf_handle("obj"), LJFAttribute::VISIBLE));
    ASSERT_THROW(ljf_environment_get(env, cast_to_ljf_handle("not_exist"), LJFAttribute::VISIBLE),
                 std::out_of_range);
}

TEST(ObjectHolder, NotEqual) {
    ObjectHolder obj1 = ljf_new_object();
    ObjectHolder obj2 = ljf_new_object();

    ASSERT_EQ(obj1, obj1);
    ASSERT_NE(obj1, obj2);
}

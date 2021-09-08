#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;

TEST(LJFSetGet, SetGetVisible)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set(obj.get(), "elem", elem.get(), ljf::VISIBLE);

    ASSERT_EQ(elem.get(), ljf_get(obj.get(), "elem", ljf::VISIBLE));
    ASSERT_THROW(ljf_get(obj.get(), "elem", ljf::HIDDEN), std::out_of_range);
}

TEST(LJFSetGet, SetGetHidden)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set(obj.get(), "elem", elem.get(), ljf::HIDDEN);

    ASSERT_EQ(elem.get(), ljf_get(obj.get(), "elem", ljf::HIDDEN));
    ASSERT_THROW(ljf_get(obj.get(), "elem", ljf::VISIBLE), std::out_of_range);
}

TEST(LJFEnvironment, GetOuterValue)
{
    using namespace ljf::internal;
    ObjectHolder obj = ljf_new_object();
    ObjectHolder env0 = create_environment();
    ljf_environment_set(env0, "obj", obj, ljf::VISIBLE);
    ObjectHolder env = create_callee_environment(env0, nullptr);
    ASSERT_EQ(obj, ljf_environment_get(env, "obj", ljf::VISIBLE));
    ASSERT_THROW(ljf_environment_get(env, "not_exist", ljf::VISIBLE), std::out_of_range);
}

TEST(ObjectHolder, NotEqual)
{
    ObjectHolder obj1 = ljf_new_object();
    ObjectHolder obj2 = ljf_new_object();

    ASSERT_EQ(obj1, obj1);
    ASSERT_NE(obj1, obj2);
}

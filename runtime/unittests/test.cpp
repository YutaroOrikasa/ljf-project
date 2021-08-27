#include "gtest/gtest.h"

#include "../runtime-internal.hpp"

using namespace ljf;

TEST(test_ljf, test_ljf_set_object_to_table)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set_object_to_table(obj.get(), "elem", elem.get());

    ASSERT_EQ(elem.get(), ljf_get_object_from_table(obj.get(), "elem"));
}

TEST(LJFGet, GetVisible)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set_object_to_table(obj.get(), "elem", elem.get());

    ASSERT_EQ(elem.get(), ljf_get(obj.get(), "elem", ljf::visible));
    ASSERT_NE(elem.get(), ljf_get(obj.get(), "elem", ljf::hidden));
}

TEST(LJFGet, GetHidden)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    ljf_set_object_to_hidden_table(obj.get(), "elem", elem.get());

    ASSERT_EQ(elem.get(), ljf_get(obj.get(), "elem", ljf::hidden));
    ASSERT_NE(elem.get(), ljf_get(obj.get(), "elem", ljf::visible));
}

TEST(ObjectHolder, NotEqual)
{
    ObjectHolder obj1 = ljf_new_object();
    ObjectHolder obj2 = ljf_new_object();

    ASSERT_EQ(obj1, obj1);
    ASSERT_NE(obj1, obj2);
}

#include "../TypeObject.hpp"
#include "gtest/gtest.h"

using namespace ljf::internal;

// FRIEND_TEST (private member access test) need defining test in namespace
namespace ljf {
TEST(calculate_type, Test) {
    auto create_obj = []() {
        ObjectHolder obj = make_new_held_object();
        ObjectHolder a1 = make_new_held_object();
        ObjectHolder a2 = make_new_held_object();
        ObjectHolder b = make_new_held_object();
        set_object_to_table(obj.get(), "a1", a1.get());
        set_object_to_table(obj.get(), "a2", a2.get());
        set_object_to_table(a1.get(), "b", b.get());
        return obj;
    };

    auto obj1 = create_obj();
    auto obj2 = create_obj();

    EXPECT_NE(obj1, obj2);
    EXPECT_EQ(obj1->calculate_type(), obj2->calculate_type());
    EXPECT_EQ(*obj1->calculate_type(), *obj2->calculate_type());
}

TEST(calculate_type, TestCircularReference) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder a = make_new_held_object();
    ObjectHolder b = make_new_held_object();

    set_object_to_table(obj.get(), "a", a.get());
    set_object_to_table(a.get(), "b", b.get());
    set_object_to_table(b.get(), "obj", obj.get());

    obj->calculate_type();

    auto b_type = b->calculate_type();
    EXPECT_FALSE(b_type->is_circular_reference());
    EXPECT_TRUE(b_type->hash_table_types_.at("obj")->is_circular_reference());
}

} // namespace ljf

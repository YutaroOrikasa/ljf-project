#include "../ObjectHolder.hpp"
#include "../ObjectIterator.hpp"
#include "gtest/gtest.h"

using namespace ljf;
using namespace ljf::internal;

static const auto ctx_up = internal::make_temporary_context();
static const auto ctx = ctx_up.get();
// unittests for Iterator family (they are private data and methods)
TEST(ObjectIterator, HashTable) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder elem = make_new_held_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto range = obj->iter_hash_table();
    auto iter = range.begin();
    auto end = range.end();
    auto kv = *iter;
    ASSERT_TRUE(iter != end);
    EXPECT_EQ(kv.key.get_key_as_c_str(), "elem");
    EXPECT_EQ(kv.value, elem);
    ++iter;
    ASSERT_TRUE(iter == end);
}

TEST(ObjectIterator, TestRange) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder elem = make_new_held_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto range = obj->iter_hash_table();
    auto iter = range.begin();
    auto end = range.end();
    auto kv = *iter;
    EXPECT_EQ(kv.key.get_key_as_c_str(), "elem");
    EXPECT_EQ(kv.value, elem);

    ASSERT_FALSE(iter == end);
    ASSERT_TRUE(iter != end);
    ++iter;
    ASSERT_TRUE(iter == end);
}

TEST(ObjectIterator, BrokenIter) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder elem = make_new_held_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto range = obj->iter_hash_table();
    auto iter = range.begin();
    EXPECT_NO_THROW(*iter);

    auto range2 = obj->iter_hash_table();
    auto iter2 = range.begin();
    set_object_to_table(obj.get(), "elem2", elem.get());

    ASSERT_THROW(*iter2, BrokenIteratorError);
}

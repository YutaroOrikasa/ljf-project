#include "../ObjectIterator.hpp"
#include "gtest/gtest.h"

using namespace ljf;

// unittests for Iterator family (they are private data and methods)
TEST(ObjectIterator, HashTable) {
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_EQ(iter.get().key, "elem");
    EXPECT_EQ(iter.get().value, elem);
    auto iter_next = iter.next();
    ASSERT_TRUE(iter_next.is_end());
}

TEST(ObjectIterator, BrokenIter) {
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_NO_THROW(iter.get());
    EXPECT_NO_THROW(iter.next());

    auto iter2 = obj->iter_hash_table();
    set_object_to_table(obj.get(), "elem2", elem.get());

    EXPECT_THROW(iter2.get(), BrokenIteratorError);
    EXPECT_THROW(iter2.next(), BrokenIteratorError);
}

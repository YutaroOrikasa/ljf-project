#include "../ObjectIterator.hpp"
#include "gtest/gtest.h"
#include "../ObjectHolder.hpp"

using namespace ljf;
using namespace ljf::internal;

static const auto ctx_up = internal::make_temporary_context();
static const auto ctx = ctx_up.get();
// unittests for Iterator family (they are private data and methods)
TEST(ObjectIterator, HashTable) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder elem = make_new_held_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_EQ(iter.get().key.get_key_as_c_str(), "elem");
    EXPECT_EQ(iter.get().value, elem);
    iter.next();
    ASSERT_TRUE(iter.is_end());
}

TEST(ObjectIterator, BrokenIter) {
    ObjectHolder obj = make_new_held_object();
    ObjectHolder elem = make_new_held_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_NO_THROW(iter.get());
    EXPECT_NO_THROW(iter.next());

    auto iter2 = obj->iter_hash_table();
    set_object_to_table(obj.get(), "elem2", elem.get());

    EXPECT_THROW(iter2.get(), BrokenIteratorError);
    EXPECT_THROW(iter2.next(), BrokenIteratorError);
}

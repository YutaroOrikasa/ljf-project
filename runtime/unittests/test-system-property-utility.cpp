#include "../AttributeTraits.hpp"
#include "../ljf-system-property.hpp"
#include "../runtime-internal.hpp"
#include "gtest/gtest.h"

using namespace ljf;
using namespace ljf::internal;

TEST(TestSystemPropertyUtility, Test) {
    auto ctx = make_temporary_context();
    ObjectWrapper obj = ljf_new(ctx.get());
    ObjectWrapper elem = ljf_new(ctx.get());
    set_ljf_system_property(obj, "ljf.elem", elem);

    ASSERT_EQ(elem, get_ljf_system_property(obj, "ljf.elem"));
    ASSERT_EQ(elem, ljf_get(ctx.get(), obj.get_wrapped_pointer(),
                            const_cast<char *>("ljf.elem"),
                            AttributeTraits::or_attr(LJFAttribute::C_STR_KEY,
                                                     LJFAttribute::HIDDEN)));
}

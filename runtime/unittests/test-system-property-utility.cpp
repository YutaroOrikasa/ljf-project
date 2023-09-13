#include "../AttributeTraits.hpp"
#include "../ljf-system-property.hpp"
#include "../runtime-internal.hpp"
#include "gtest/gtest.h"

using namespace ljf;
using namespace ljf::internal;

TEST(TestSystemPropertyUtility, Test) {
    ObjectWrapper obj = make_new_wrapped_object();
    ObjectWrapper elem = make_new_wrapped_object();
    set_ljf_system_property(obj, "ljf.elem", elem);

    ASSERT_EQ(elem, get_ljf_system_property(obj, "ljf.elem"));
    ASSERT_EQ(elem, obj.get_hidden("ljf.elem"));
}

#include "gtest/gtest.h"

#include "../ljf-system-property.hpp"
#include "../runtime-internal.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;
using namespace ljf::internal;

static const char *cast_native_data_to_c_str(native_data_t data) {
    return const_cast<const char *>(reinterpret_cast<char *>(data));
}

TEST(LJFCStrWrapper, Test) {
    auto ctx = make_temporary_context();
    auto c_str = "hello";
    auto c_str_wrapper_handle = ljf_wrap_c_str(ctx.get(), c_str);
    auto c_str_wrapper = ctx->get_from_handle(c_str_wrapper_handle);

    ASSERT_EQ(c_str, cast_native_data_to_c_str(c_str_wrapper->get_native_data()));
}

TEST(LJFCStrWrapper, TestLength) {
    auto ctx = make_temporary_context();
    auto c_str = "hello";
    auto c_str_wrapper = ljf_wrap_c_str(ctx.get(), c_str);
    auto c_str_obj = ObjectWrapper(ctx->get_from_handle(c_str_wrapper));

    auto len_obj = get_ljf_system_property(c_str_obj, ljf_c_str_length);
    auto len = len_obj.get_wrapped_pointer()->get_native_data();
    ASSERT_EQ(strlen(c_str), len);
}

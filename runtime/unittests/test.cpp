#include "gtest/gtest.h"

#include "../runtime-internal.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/internal/ObjectHolder.hpp"
#include "ljf/runtime.hpp"

using namespace ljf;
using namespace ljf::internal;

namespace
{
struct LJFSetGet : public ::testing::Test {
    ObjectWrapper obj = make_new_wrapped_object();
    ObjectWrapper elem = make_new_wrapped_object();
};
} // namespace

namespace
{
struct LJFEnvironment : public ::testing::Test {
    std::unique_ptr<ljf::Context> ctx = make_temporary_context();
    ObjectHolder obj = make_new_held_object();
    ObjectHolder env0 = create_environment(ctx.get());
    ObjectHolder env = create_callee_environment(env0, nullptr);
};
} // namespace


TEST_F(LJFSetGet, SetGetVisible) {
     obj.set("elem", elem);

    ASSERT_EQ(elem, obj.get("elem"));
    ASSERT_THROW(obj.get_hidden("elem"), std::out_of_range);
}

TEST_F(LJFSetGet, SetGetHidden) {
    obj.set_hidden("elem", elem);

    ASSERT_EQ(elem, obj.get_hidden("elem"));

    ASSERT_THROW(obj.get("elem"), std::out_of_range);
}

TEST_F(LJFEnvironment, GetOuterValue) {


    ljf_environment_set(ctx.get(), env0, cast_to_ljf_handle("obj"),
                        obj.get_handle(*ctx), LJF_ATTR_VISIBLE);

    ASSERT_EQ(obj,
              ctx->get_from_handle(ljf_environment_get(ctx.get(), env, cast_to_ljf_handle("obj"),
                                  LJF_ATTR_VISIBLE)));
}

TEST_F(LJFEnvironment, GetNotExistValue) {

    ASSERT_THROW(ljf_environment_get(ctx.get(), env,
                                     cast_to_ljf_handle("not_exist"),
                                     LJF_ATTR_VISIBLE),
                 std::out_of_range);
}

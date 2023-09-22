#include "ljf/ObjectWrapper.hpp"

#include "AttributeTraits.hpp"
#include "Object.hpp"

namespace ljf {
ObjectWrapper ObjectWrapper::get_impl(const char *key,
                                      LJFAttribute visiblity) const {
    return ObjectHolder(holder_.get()->get(
        key,
        AttributeTraits::or_attr(LJF_ATTR_C_STR_KEY, visiblity)));
}

void ObjectWrapper::set_impl(const char *key, const ObjectWrapper &value,
                             LJFAttribute visiblity) const {
    holder_.get()->set(
        key, cast_object_to_ObjectPtrOrNativeValue(value.holder_.get()),
        AttributeTraits::or_attr(LJF_ATTR_C_STR_KEY, visiblity));
}

ObjectWrapper make_new_wrapped_object() { return ObjectWrapper(new Object()); }

} // namespace ljf

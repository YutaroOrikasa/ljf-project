#include "ljf/ObjectWrapper.hpp"

#include "AttributeTraits.hpp"
#include "Object.hpp"

namespace ljf {
ObjectWrapper ObjectWrapper::get_impl(const char *key,
                                      LJFAttribute visiblity) const {
    return ObjectHolder(holder_.get()->get(
        const_cast<char *>(key),
        AttributeTraits::or_attr(LJFAttribute::C_STR_KEY, visiblity)));
}

void ObjectWrapper::set_impl(const char *key, const ObjectWrapper &value,
                             LJFAttribute visiblity) const {
    holder_.get()->set(
        const_cast<char *>(key), cast_object_to_ObjectPtrOrNativeValue(value.holder_.get()),
        AttributeTraits::or_attr(LJFAttribute::C_STR_KEY, visiblity));
}

uint64_t ObjectWrapper::get_native_value(const char *key,
                                         LJFAttribute attr) const {
    return static_cast<uint64_t>(holder_->get(key, attr));
}

void ObjectWrapper::set_native_value(const char *key, uint64_t value,
                                     LJFAttribute attr) const {
    throw "stab";
    // holder_->set(key, value, attr);
}

ObjectWrapper make_new_wrapped_object() { return ObjectWrapper(new Object()); }

} // namespace ljf

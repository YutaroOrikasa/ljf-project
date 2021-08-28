#include "runtime-internal.hpp"
#include "Object.hpp"

namespace ljf
{

ObjectHolder::ObjectHolder(Object *o) noexcept : obj_(o)
{
    increment_ref_count(obj_);
}

ObjectHolder::ObjectHolder(const ObjectHolder &other) noexcept : ObjectHolder(other.obj_) {}

// On move we don't have to operate refcount
ObjectHolder::ObjectHolder(ObjectHolder &&other) noexcept : obj_(other.obj_)
{
    other.obj_ = nullptr;
}

ObjectHolder &ObjectHolder::operator=(Object *o) noexcept
{
    decrement_ref_count(obj_);
    obj_ = o;
    increment_ref_count(obj_);
    return *this;
}

ObjectHolder &ObjectHolder::operator=(const ObjectHolder &other) noexcept
{
    return (*this = other.obj_);
}
ObjectHolder &ObjectHolder::operator=(ObjectHolder &&other) noexcept
{
    decrement_ref_count(obj_);
    obj_ = other.obj_;
    other.obj_ = nullptr;
    return *this;
}

ObjectHolder::~ObjectHolder()
{
    // std::cerr << "ObjectHolder::~ObjectHolder() this: " << this << ", obj_: " << obj_ << std::endl;
    decrement_ref_count(obj_);
}
} // namespace ljf

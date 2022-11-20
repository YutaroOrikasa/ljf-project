#pragma once
#include "Object.hpp"

namespace ljf {

class ObjectHolder {
    Object *obj_ = nullptr;

public:
    ObjectHolder() = default;

    /*implicit*/ ObjectHolder(Object *o) noexcept : obj_(o) {
        increment_ref_count(obj_);
    }

    /*implicit*/ ObjectHolder(const ObjectHolder &other) noexcept
        : ObjectHolder(other.obj_) {}

    /*implicit*/ ObjectHolder(ObjectHolder &&other) noexcept
        : obj_(other.obj_) {
        // On move we don't have to operate refcount
        other.obj_ = nullptr;
    }

    ObjectHolder &operator=(Object *o) noexcept {
        // Consider case obj_ == o
        increment_ref_count(o);
        decrement_ref_count(obj_);
        obj_ = o;
        return *this;
    }

    ObjectHolder &operator=(const ObjectHolder &other) noexcept {
        return (*this = other.obj_);
    }

    ObjectHolder &operator=(ObjectHolder &&other) noexcept {
        decrement_ref_count(obj_);
        obj_ = other.obj_;
        other.obj_ = nullptr;
        return *this;
    }

    ~ObjectHolder() {
        // std::cerr << "ObjectHolder::~ObjectHolder() this: " << this << ",
        // obj_: " << obj_ << std::endl;
        decrement_ref_count(obj_);
    }

    Object *get() const noexcept { return obj_; }

    /*implicit*/ operator Object *() const noexcept { return obj_; }

    explicit operator bool() const noexcept { return bool(obj_); }

    Object *operator->() const noexcept { return obj_; }

    Object &operator*() const noexcept { return *obj_; }

    // operator Object *() defined so operator== and != need not be defined.
};
} // namespace ljf

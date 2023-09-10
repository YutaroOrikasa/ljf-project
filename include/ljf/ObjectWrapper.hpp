#pragma once

#include "internal/ObjectHolder.hpp"
#include "ljf/runtime.hpp"

namespace ljf {

class ObjectWrapper {
private:
    ObjectHolder holder_;

    ObjectWrapper get_impl(const char *key, LJFAttribute visiblity) const;

    void set_impl(const char *key, const ObjectWrapper &value,
                  LJFAttribute visiblity) const;

public:
    /*implicit*/ ObjectWrapper(ObjectHolder holder) : holder_(holder) {}
    /*implicit*/ ObjectWrapper(Object *obj) : holder_(obj) {}

    ObjectWrapper() = default;
    ObjectWrapper(const ObjectWrapper &) = default;
    ObjectWrapper(ObjectWrapper &&) = default;
    ObjectWrapper &operator=(const ObjectWrapper &) = default;
    ObjectWrapper &operator=(ObjectWrapper &&) = default;

    bool operator==(const ObjectWrapper &other) const {
        return holder_ == other.holder_;
    }

    bool operator!=(const ObjectWrapper &other) const {
        return holder_ != other.holder_;
    }

    /// @brief Please use carefully (eg. dangling returned pointer).
    /// @return
    Object *get_wrapped_pointer() const { return holder_.get(); }

    ObjectWrapper get(const char *key) const {
        return get_impl(key, LJFAttribute::VISIBLE);
    }

    ObjectWrapper get_hidden(const char *key) const {
        return get_impl(key, LJFAttribute::HIDDEN);
    }

    void set(const char *key, const ObjectWrapper &value) const {
        set_impl(key, value, LJFAttribute::VISIBLE);
    }
    void set_hidden(const char *key, const ObjectWrapper &value) const {
        set_impl(key, value, LJFAttribute::HIDDEN);
    }
};

} // namespace ljf

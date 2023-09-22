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

    /// @brief Please use carefully (eg. getting native value as object pointer).
    /// @return
    uint64_t get_native_value(const char *key, LJFAttribute attr) const;

    /// @brief Please use carefully (eg. passing invalid attribute).
    /// @return
    void set_native_value(const char *key, uint64_t value,
                          LJFAttribute attr) const;

    ObjectWrapper get(const char *key) const {
        return get_impl(key, LJF_ATTR_VISIBLE);
    }

    ObjectWrapper get_hidden(const char *key) const {
        return get_impl(key, LJF_ATTR_HIDDEN);
    }

    void set(const char *key, const ObjectWrapper &value) const {
        set_impl(key, value, LJF_ATTR_VISIBLE);
    }
    void set_hidden(const char *key, const ObjectWrapper &value) const {
        set_impl(key, value, LJF_ATTR_HIDDEN);
    }
};

ObjectWrapper make_new_wrapped_object();

} // namespace ljf

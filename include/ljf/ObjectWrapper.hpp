#pragma once

#include "internal/ObjectHolder.hpp"

namespace ljf {

class ObjectWrapper {
private:
    ObjectHolder holder_;

public:
    /*implicit*/ ObjectWrapper(ObjectHolder holder) : holder_(holder) {}
    /*implicit*/ ObjectWrapper(Object *obj) : holder_(obj) {}

    ObjectWrapper() = default;
    ObjectWrapper(const ObjectWrapper &) = default;
    ObjectWrapper(ObjectWrapper &&) = default;
    ObjectWrapper &operator=(const ObjectWrapper &) = default;
    ObjectWrapper &operator=(ObjectWrapper &&) = default;

    ObjectWrapper get(const char *key) const;
};

} // namespace ljf

#pragma once

#include "ljf/ObjectWrapper.hpp"

namespace ljf::internal {

inline ObjectWrapper get_ljf_system_property(const ObjectWrapper &obj,
                                             const char *key) {

    return obj.get_hidden(key);
}

inline void set_ljf_system_property(const ObjectWrapper &obj, const char *key,
                                    const ObjectWrapper &value) {

    obj.set_hidden(key, value);
}

} // namespace ljf::internal

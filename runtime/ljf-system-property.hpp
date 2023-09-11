#pragma once

#include "ljf/ObjectWrapper.hpp"
#include "ljf/runtime.hpp"

namespace ljf::internal {

constexpr auto ljf_native_value_int64 = "ljf.native_value_int64";

constexpr auto ljf_native_value_c_str = "ljf.native_value_c_str";
constexpr auto ljf_c_str_length = "ljf.c_str_length";

inline LJFHandle get_ljf_native_system_property(const ObjectWrapper &obj,
                                                const char *key) {

    return obj.get_native_value(key, LJFAttribute::VISIBLE /* LJF_ATTR_C_STR_KEY | LJF_ATTR_HIDDEN |
                                   LJF_ATTR_NATIVE */);
}

inline void set_ljf_native_system_property(const ObjectWrapper &obj,
                                           const char *key, LJFHandle handle) {

    obj.set_native_value(
        key, handle, LJFAttribute::HIDDEN
        /* LJF_ATTR_C_STR_KEY | LJF_ATTR_HIDDEN | LJF_ATTR_NATIVE */);
}

inline ObjectWrapper get_ljf_system_property(const ObjectWrapper &obj,
                                             const char *key) {

    return obj.get_hidden(key);
}

inline void set_ljf_system_property(const ObjectWrapper &obj, const char *key,
                                    const ObjectWrapper &value) {

    obj.set_hidden(key, value);
}

} // namespace ljf::internal

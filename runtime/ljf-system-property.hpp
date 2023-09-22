#pragma once

#include "Object.hpp"
#include "ljf/ObjectWrapper.hpp"
#include "ljf/runtime.hpp"

namespace ljf::internal {

constexpr auto ljf_native_value_int64 = "ljf.native_value_int64";

constexpr auto ljf_native_value_c_str = "ljf.native_value_c_str";
constexpr auto ljf_c_str_length = "ljf.c_str_length";

inline native_data_t get_ljf_native_system_property(const ObjectWrapper &obj,
                                                    const char *key) {

    auto data_obj = obj.get_hidden(key);
    return data_obj.get_wrapped_pointer()->get_native_data();
}

inline void set_ljf_native_system_property(const ObjectWrapper &obj,
                                           const char *key,
                                           native_data_t data) {

    auto data_obj = make_new_wrapped_object(data);

    obj.set_hidden(key, data_obj);
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

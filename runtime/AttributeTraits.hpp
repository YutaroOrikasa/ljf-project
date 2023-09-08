#pragma once

#include "ljf/runtime.hpp"

namespace ljf {

struct AttributeTraits {
private:
    static bool is_flag_enable(LJFAttribute attr, int64_t mask) {
        return bool(static_cast<uint64_t>(attr) & mask);
    }

public:
    static LJFAttribute mask(LJFAttribute a, LJFAttribute b) {
        return static_cast<LJFAttribute>( static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
    }
    static bool is_visible(LJFAttribute attr) {
        return !is_flag_enable(attr, 0b0001);
    }
};

} // namespace ljf

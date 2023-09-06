#pragma once

#include "ljf/runtime.hpp"

namespace ljf {

struct AttributeTraits {
private:
    static bool is_flag_enable(LJFAttribute attr, int64_t mask) {
        return bool(static_cast<uint64_t>(attr) & mask);
    }

public:
    static bool is_visible(LJFAttribute attr) {
        return !is_flag_enable(attr, 0b0001);
    }
};

} // namespace ljf

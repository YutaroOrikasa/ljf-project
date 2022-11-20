#pragma once

// User's custom configuration is implicitly incluted by CONFIG_FILE Makefile
// variable and -include clang option.

#if !defined(LJF_CALCULATE_TYPE)
#define LJF_CALCULATE_TYPE false
#endif // LJF_CALCULATE_TYPE

namespace ljf::config {
static constexpr bool calculate_type = LJF_CALCULATE_TYPE;
#undef LJF_CALCULATE_TYPE
} // namespace ljf::config

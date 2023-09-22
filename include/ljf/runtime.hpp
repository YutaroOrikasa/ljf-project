#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>

using LJFHandle = uint64_t;
namespace ljf {
class Object;
class Context;
using native_data_t = uint64_t;
using Environment = Object;
using FunctionId = std::size_t;

using FunctionPtr = LJFHandle (*)(Context *ctx, Environment *);

enum TableVisiblity {
    TAB_VISIBLE,
    TAB_HIDDEN,
    VISIBLE = TAB_VISIBLE,
    HIDDEN = TAB_HIDDEN
};

class runtime_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

inline LJFHandle cast_to_ljf_handle(const char *str) {
    return reinterpret_cast<LJFHandle>(const_cast<char *>(str));
}

template <typename T> LJFHandle cast_to_ljf_handle(T value) {
    return reinterpret_cast<LJFHandle>(static_cast<uint64_t>(value));
}

template <typename Out> Out &operator<<(Out &out, const Object &);
} // namespace ljf

// LJFAttribute is defined as const integer, not enum
// because bitwise operation is not allowed with enum in c++.
using LJFAttribute = uint64_t;

constexpr LJFAttribute LJF_ATTR_KEY_ATTR_MASK = UINT32_MAX;
constexpr LJFAttribute LJF_ATTR_DEFAULT = 0;
// visiblity, bit 0
constexpr LJFAttribute LJF_ATTR_VISIBLE = 0 << 0;
constexpr LJFAttribute LJF_ATTR_HIDDEN = 1 << 0;
//
// key type, bit 3, 4
constexpr LJFAttribute LJF_ATTR_KEY_TYPE_MASK = 0b11 << 3;
constexpr LJFAttribute LJF_ATTR_C_STR_KEY = 0 << 3;
constexpr LJFAttribute LJF_ATTR_OBJECT_KEY = 1 << 3;
//
//
constexpr LJFAttribute LJF_ATTR_VALUE_ATTR_MASK = (unsigned long)(UINT32_MAX)
                                                  << 32;
// data type, bit 32
// Unboxed object is made by JIT/AOT compiler (future plan)
constexpr LJFAttribute LJF_ATTR_DATA_TYPE_MASK = 1ul << 32;
constexpr LJFAttribute LJF_ATTR_BOXED_OBJECT = 0ul << 32;
constexpr LJFAttribute LJF_ATTR_UNBOXED_OBJECT = 1ul << 32;

//
// constant type, bit 33, 34
constexpr LJFAttribute LJF_ATTR_MUTABLE = 0ul << 33;
constexpr LJFAttribute LJF_ATTR_MAYBE_CONSTANT = 1ul << 33;
// reserved for future:
// CONSTANT = 0b10 << 33

extern "C" {
LJFHandle ljf_get(ljf::Context *, LJFHandle obj, LJFHandle key,
                  LJFAttribute attr);

void ljf_set(ljf::Context *, LJFHandle obj, LJFHandle key, LJFHandle value,
             LJFAttribute attr);

/**************** function API ***************/
ljf::FunctionId ljf_get_function_id_from_function_table(ljf::Object *obj,
                                                        const char *key);
void ljf_set_function_id_to_function_table(ljf::Object *obj, const char *key,
                                           ljf::FunctionId function_id);

LJFHandle ljf_call_function(ljf::Context *, ljf::FunctionId function_id,
                            LJFHandle env, LJFHandle arg);

/**************** new API ***************/
LJFHandle ljf_new(ljf::Context *);

LJFHandle ljf_new_with_native_data(ljf::Context *ctx, ljf::native_data_t data);

uint64_t ljf_get_native_data(const ljf::Object *obj);

/**************** environment API ***************/
LJFHandle ljf_environment_get(ljf::Context *, ljf::Environment *env,
                              LJFHandle key, LJFAttribute attr);
void ljf_environment_set(ljf::Context *, ljf::Environment *env, LJFHandle key,
                         LJFHandle value, LJFAttribute attr);

/**************** function registration API ***************/
ljf::FunctionId ljf_register_native_function(ljf::FunctionPtr);

/**************** array API ***************/
size_t ljf_array_size(ljf::Context *, LJFHandle obj);
LJFHandle ljf_array_get(ljf::Context *, LJFHandle obj, size_t index);
void ljf_array_set(ljf::Object *obj, size_t index, ljf::Object *value);
void ljf_array_push(ljf::Context *, LJFHandle obj, LJFHandle value);

/**************** other API ***************/
LJFHandle ljf_import(ljf::Context *, const char *src_path,
                     const char *language);
LJFHandle ljf_wrap_c_str(ljf::Context *, const char *str);
}

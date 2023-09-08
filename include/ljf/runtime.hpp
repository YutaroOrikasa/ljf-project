#pragma once

#include <any>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace ljf {
class Object;
class Context;
using Environment = Object;
using FunctionId = std::size_t;

using FunctionPtr = Object *(*)(Context *ctx, Environment *);

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

template <typename Out> Out &operator<<(Out &out, const Object &);
} // namespace ljf

enum class LJFAttribute : uint64_t {
    // visiblity, bit 0
    VISIBLE = 0 << 0,
    HIDDEN = 1 << 0,
    // constant type, bit 1, 2
    MUTABLE = 0 << 1,
    MAYBE_CONSTANT = 1 << 1,
    // reserved for future:
    // CONSTANT = 0b10 << 1
    //
    // key type, bit 3, 4
    KEY_TYPE_MASK = 0b11 << 3,
    C_STR_KEY = 0 << 3,
    OBJECT_KEY = 1 << 3,
    //
    // data type, bit 32
    OBJECT = 0ul << 32,
    NATIVE = 1ul << 32,
};

constexpr ljf::Object *ljf_undefined = nullptr;

extern "C" {
ljf::Object *ljf_get(ljf::Object *obj, const char *key, LJFAttribute attr);

void ljf_set(ljf::Object *obj, const char *key, ljf::Object *value,
             LJFAttribute attr);

/**************** function API ***************/
ljf::FunctionId ljf_get_function_id_from_function_table(ljf::Object *obj,
                                                        const char *key);
void ljf_set_function_id_to_function_table(ljf::Object *obj, const char *key,
                                           ljf::FunctionId function_id);

ljf::Object *ljf_call_function(ljf::Context *, ljf::FunctionId function_id,
                               ljf::Object *env, ljf::Object *arg);

/**************** new API ***************/
ljf::Object *ljf_new(ljf::Context *);
ljf::Object *ljf_new_object();

ljf::Object *ljf_new_object_with_native_data(uint64_t data);

uint64_t ljf_get_native_data(const ljf::Object *obj);

/**************** environment API ***************/
ljf::Object *ljf_environment_get(ljf::Environment *env, const char *key,
                                 LJFAttribute attr);
void ljf_environment_set(ljf::Environment *env, const char *key,
                         ljf::Object *value, LJFAttribute attr);

/**************** function registration API ***************/
ljf::FunctionId ljf_register_native_function(ljf::FunctionPtr);

/**************** array API ***************/
size_t ljf_array_size(ljf::Object *obj);
ljf::Object *ljf_array_get(ljf::Object *obj, size_t index);
void ljf_array_set(ljf::Object *obj, size_t index, ljf::Object *value);
void ljf_array_push(ljf::Object *obj, ljf::Object *value);

/**************** other API ***************/
ljf::Object *ljf_import(ljf::Context *, const char *src_path,
                        const char *language);
ljf::Object *ljf_wrap_c_str(const char *str);
}

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

enum TableVisiblity { VISIBLE, HIDDEN };

class runtime_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <typename Out> Out &operator<<(Out &out, const Object &);
} // namespace ljf

constexpr ljf::Object *ljf_undefined = nullptr;

extern "C" {
ljf::Object *ljf_get(ljf::Object *obj, const char *key,
                     ljf::TableVisiblity visiblity);

void ljf_set(ljf::Object *obj, const char *key, ljf::Object *value,
             ljf::TableVisiblity visiblity);

/**************** function API ***************/
ljf::FunctionId ljf_get_function_id_from_function_table(ljf::Object *obj,
                                                        const char *key);
void ljf_set_function_id_to_function_table(ljf::Object *obj, const char *key,
                                           ljf::FunctionId function_id);

ljf::Object *ljf_call_function(ljf::Context *, ljf::FunctionId function_id,
                               ljf::Object *env, ljf::Object *arg);

/**************** new API ***************/
ljf::Object *ljf_new_object();

ljf::Object *ljf_new_object_with_native_data(uint64_t data);


uint64_t ljf_get_native_data(const ljf::Object *obj);

/**************** environment API ***************/
ljf::Object *ljf_environment_get(ljf::Environment *env, const char *key,
                                 ljf::TableVisiblity vis);
void ljf_environment_set(ljf::Environment *env, const char *key,
                         ljf::Object *value, ljf::TableVisiblity vis);

/**************** function registration API ***************/
ljf::FunctionId ljf_register_native_function(ljf::FunctionPtr);

/**************** array API ***************/
size_t ljf_array_size(ljf::Object *obj);
ljf::Object *ljf_array_get(ljf::Object *obj, size_t index);
void ljf_array_set(ljf::Object *obj, size_t index, ljf::Object *value);
void ljf_array_push(ljf::Object *obj, ljf::Object *value);

ljf::Object *ljf_wrap_c_str(const char *str);
}

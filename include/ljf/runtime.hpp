#pragma once

#include <any>
#include <memory>
#include <unordered_map>
#include <cassert>
#include <cstdint>

namespace ljf
{
class Object;
using Environment = Object;

enum TableVisiblity
{
    visible,
    hidden
};

class runtime_error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

template <typename Out>
Out &operator<<(Out &out, const Object &);
} // namespace ljf

using LJFFunctionId = std::size_t;

using LJFObject = ljf::Object;

constexpr std::nullptr_t ljf_undefined = nullptr;

extern "C"
{
    LJFObject *ljf_get(ljf::Object *obj, const char *key, ljf::TableVisiblity visiblity);

    void ljf_set_object_to_table(LJFObject *obj, const char *key, LJFObject *value);

    LJFObject *ljf_get_object_from_hidden_table(LJFObject *obj, const char *key);
    void ljf_set_object_to_hidden_table(LJFObject *obj, const char *key, LJFObject *value);

    LJFFunctionId ljf_get_function_id_from_function_table(LJFObject *obj, const char *key);
    void ljf_set_function_id_to_function_table(LJFObject *obj, const char *key, LJFFunctionId function_id);

    void ljf_push_object_to_array(LJFObject *obj, LJFObject *value);

    LJFObject *ljf_call_function(LJFFunctionId function_id, LJFObject *env, LJFObject *arg);

    LJFObject *ljf_new_object();

    LJFObject *ljf_new_object_with_native_data(uint64_t data);
    uint64_t ljf_get_native_data(const LJFObject *obj);

    LJFObject *ljf_get_object_from_environment(ljf::Environment *env, const char *key);
    void ljf_set_object_to_environment(ljf::Environment *env, const char *key, LJFObject *value);

    LJFFunctionId ljf_register_native_function(LJFObject *(*fn)(LJFObject *env, LJFObject *tmp));

    size_t ljf_array_size(LJFObject *obj);
    LJFObject *ljf_get_object_from_array(LJFObject *obj, size_t index);

    LJFObject *ljf_wrap_c_str(const char *str);
}

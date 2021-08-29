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
using FunctionId = std::size_t;
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


constexpr std::nullptr_t ljf_undefined = nullptr;

extern "C"
{
    ljf::Object *ljf_get(ljf::Object *obj, const char *key, ljf::TableVisiblity visiblity);

    void ljf_set(ljf::Object *obj, const char *key, ljf::Object *value, ljf::TableVisiblity visiblity);


    ljf::FunctionId ljf_get_function_id_from_function_table(ljf::Object *obj, const char *key);
    void ljf_set_function_id_to_function_table(ljf::Object *obj, const char *key, ljf::FunctionId function_id);

    ljf::Object *ljf_call_function(ljf::FunctionId function_id, ljf::Object *env, ljf::Object *arg);

    ljf::Object *ljf_new_object();

    ljf::Object *ljf_new_object_with_native_data(uint64_t data);
    uint64_t ljf_get_native_data(const ljf::Object *obj);

    ljf::Object *ljf_get_object_from_environment(ljf::Environment *env, const char *key);
    void ljf_set_object_to_environment(ljf::Environment *env, const char *key, ljf::Object *value);

    ljf::FunctionId ljf_register_native_function(ljf::Object *(*fn)(ljf::Object *env, ljf::Object *tmp));

    /**************** array API ***************/
    size_t ljf_array_size(ljf::Object *obj);
    ljf::Object *ljf_array_get(ljf::Object *obj, size_t index);
    void ljf_array_set(ljf::Object *obj, size_t index, ljf::Object *value);
    void ljf_array_push(ljf::Object *obj, ljf::Object *value);


    ljf::Object *ljf_wrap_c_str(const char *str);
}

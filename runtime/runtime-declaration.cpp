#include <ljf/runtime.hpp>

extern "C" void ljf_dummy(void (*touch)(...))
{
    touch(ljf_get,
          ljf_set,
          ljf_get_function_id_from_function_table, ljf_set_function_id_to_function_table,
          ljf_call_function,
          ljf_new_object,
          ljf_new_object_with_native_data,
          ljf_get_native_data,
          ljf_environment_get,
          ljf_environment_set,
          ljf_register_native_function,
          ljf_array_size,
          ljf_array_set,
          ljf_array_push,
          ljf_wrap_c_str);
}

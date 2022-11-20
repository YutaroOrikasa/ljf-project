#pragma once

#include "ObjectHolder.hpp"
#include <ljf/ljf.hpp>
#include <ljf/runtime.hpp>

namespace llvm {
class Function;
} // namespace llvm

namespace ljf::internal {

ObjectHolder create_environment(bool prepare_0th_frame = true);
ObjectHolder create_callee_environment(Environment *parent, Object *arg);

ObjectHolder load_source_code(const std::string &language,
                              const std::string &source_path, Object *env);
} // namespace ljf::internal

extern "C" {
ljf::FunctionId ljf_internal_register_llvm_function(llvm::Function *f);

ljf::Object *ljf_internal_get_object_by_index(ljf::Object *obj, uint64_t index);
void ljf_internal_set_object_by_index(ljf::Object *obj, uint64_t index,
                                      ljf::Object *value);
void ljf_internal_reserve_object_array_table_size(ljf::Object *obj,
                                                  uint64_t size);
void ljf_internal_resize_object_array_table_size(ljf::Object *obj,
                                                 uint64_t size);
}

namespace ljf {
// called with dlopen and dlsym
typedef void (*ljf_internal_initialize_t)(const CompilerMap &compiler_map,
                                          const std::string &ljf_tmpdir,
                                          const std::string &runtime_filename);
typedef int (*ljf_internal_start_entry_point_t)(ljf_main_t ljf_main,
                                                const std::string &language,
                                                const std::string &source_path,
                                                int argc, const char **argv);
} // namespace ljf

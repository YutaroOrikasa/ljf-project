#pragma once

#include <ljf/ljf.hpp>
#include <ljf/runtime.hpp>

namespace llvm
{
class Function;
} // namespace llvm

namespace ljf
{

class ObjectHolder
{
    Object *obj_ = nullptr;

public:
    ObjectHolder() = default;
    // implicit
    ObjectHolder(Object *o) noexcept;
    // implicit
    ObjectHolder(const ObjectHolder &other) noexcept;
    // implicit
    ObjectHolder(ObjectHolder &&other) noexcept;

    ObjectHolder &operator=(Object *o) noexcept;
    ObjectHolder &operator=(const ObjectHolder &other) noexcept;
    ObjectHolder &operator=(ObjectHolder &&other) noexcept;

    ~ObjectHolder();

    // inline member functions

    Object *get() const noexcept
    {
        return obj_;
    }

    /*implicit*/ operator Object *() const noexcept
    {
        return obj_;
    }

    explicit operator bool() const noexcept
    {
        return bool(obj_);
    }

    Object *operator->() const noexcept
    {
        return obj_;
    }

    Object &operator*() const noexcept
    {
        return *obj_;
    }

    // operator Object *() defined so operator== and != need not be defined.
};
} // namespace ljf

namespace ljf::internal
{

ObjectHolder create_environment(bool prepare_0th_frame = true);
ObjectHolder create_callee_environment(Environment *parent, Object *arg);

ObjectHolder load_source_code(const std::string &language, const std::string &source_path, Object *env);
} // namespace ljf::internal

extern "C"
{
    ljf::FunctionId ljf_internal_register_llvm_function(llvm::Function *f);

    ljf::Object *ljf_internal_get_object_by_index(ljf::Object *obj, uint64_t index);
    void ljf_internal_set_object_by_index(ljf::Object *obj, uint64_t index, ljf::Object *value);
    void ljf_internal_reserve_object_array_table_size(ljf::Object *obj, uint64_t size);
    void ljf_internal_resize_object_array_table_size(ljf::Object *obj, uint64_t size);
}

namespace ljf
{
// called with dlopen and dlsym
typedef void (*ljf_internal_initialize_t)(const CompilerMap &compiler_map, const std::string &ljf_tmpdir, const std::string &runtime_filename);
typedef int (*ljf_internal_start_entry_point_t)(ljf_main_t ljf_main,
                                                const std::string &language, const std::string &source_path,
                                                int argc, const char **argv);
} // namespace ljf

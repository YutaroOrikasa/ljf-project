#pragma once

#include <ljf/runtime.hpp>

namespace llvm
{
    class Function;
} // namespace llvm


namespace ljf
{

    class ObjectHolder {
        Object *obj_ = nullptr;
    public:
        ObjectHolder() = default;
        // implicit
        ObjectHolder(Object* o) noexcept;
        // implicit
        ObjectHolder(const ObjectHolder &other) noexcept;
        // implicit
        ObjectHolder(ObjectHolder &&other) noexcept;

        ObjectHolder& operator=(Object* o) noexcept;
        ObjectHolder& operator=(const ObjectHolder &other) noexcept;
        ObjectHolder& operator=(ObjectHolder &&other) noexcept;

        ~ObjectHolder();


        // inline member functions

        Object* get() const noexcept
        {
            return obj_;
        }

        operator bool() const noexcept
        {
            return bool(obj_);
        }

        Object* operator->() const noexcept
        {
            return obj_;
        }

        Object& operator*() const noexcept
        {
            return *obj_;
        }

        bool operator==(const ObjectHolder &rhs)
        {
            return get() == rhs.get();
        }

        bool operator!=(const ObjectHolder &rhs)
        {
            return get() != rhs.get();
        }
    };
} // namespace ljf

namespace ljf::internal
{

    ObjectHolder create_environment(bool prepare_0th_frame=true);
}

extern "C"
{
    LJFFunctionId ljf_internal_register_llvm_function(llvm::Function* f);

    LJFObject *ljf_internal_get_object_by_index(LJFObject *obj, uint64_t index);
    void ljf_internal_set_object_by_index(LJFObject *obj, uint64_t index, LJFObject *value);
    void ljf_internal_reserve_object_array_table_size(LJFObject *obj, uint64_t size);
    void ljf_internal_resize_object_array_table_size(LJFObject *obj, uint64_t size);
}

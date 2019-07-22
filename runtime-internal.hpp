#pragma once

#include <ljf/runtime.hpp>

namespace llvm
{
    class Function;
} // namespace llvm


namespace ljf
{
    LJFFunctionId register_llvm_function(llvm::Function* f);

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

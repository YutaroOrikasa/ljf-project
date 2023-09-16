#pragma once
namespace ljf {

class Object;
enum class ObjectPtrOrNativeValue : uint64_t {};
enum class IncrementedObjectPtrOrNativeValue : uint64_t {
    NULL_PTR = 0
};

inline ObjectPtrOrNativeValue cast_object_to_ObjectPtrOrNativeValue(Object *obj) {
    return static_cast<ObjectPtrOrNativeValue>(reinterpret_cast<uint64_t>(obj));
}

void increment_ref_count(Object *obj);
void decrement_ref_count(Object *obj);
} // namespace ljf

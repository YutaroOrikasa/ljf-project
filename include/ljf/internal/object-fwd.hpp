#pragma once
namespace ljf {

class Object;
enum class ObjectPtrOrNativeValue : uint64_t {};
enum class IncrementedObjectPtrOrNativeValue : uint64_t {
    NULL_PTR = 0
};

void increment_ref_count(Object *obj);
void decrement_ref_count(Object *obj);
} // namespace ljf

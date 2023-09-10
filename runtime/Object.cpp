#include "Object.hpp"

namespace ljf {

void increment_ref_count(Object *obj) {
    if (obj) {
        std::lock_guard lk{*obj};
        obj->ref_count_++;
    }
}

void decrement_ref_count(Object *obj) {
    // std::cerr << "decrement_ref_count() obj: " << obj << std::endl;

    if (!obj) {
        return;
    }

    obj->lock();
    assert(obj->ref_count_ > 0);
    obj->ref_count_--;
    if (obj->ref_count_ == 0) {
        obj->unlock();
        delete obj;

        // Commented out for experimentation.
        //
        // assert(allocated_memory_size >= sizeof(Object));
        // allocated_memory_size -= sizeof(Object);
        return;
    }
    obj->unlock();
}

namespace internal {
    ObjectHolder make_new_held_object() { return ObjectHolder(new Object()); }
} // namespace internal

} // namespace ljf

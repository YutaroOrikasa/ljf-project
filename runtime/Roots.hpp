#pragma once

#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Object.hpp"
#include "ObjectHolder.hpp"
#include "ljf/ObjectWrapper.hpp"

namespace ljf {

class ThreadLocalRoot;

class GlobalRoot {
private:
    std::mutex mutex;
    std::unordered_map<std::thread::id, ThreadLocalRoot *> threads;
    ObjectHolder global_root_object_;

public:
    void add_thread(std::thread::id id, ThreadLocalRoot *thread) {

        std::lock_guard lk{mutex};
        threads[id] = thread;
    }

    void erase_thread(std::thread::id id) {
        std::lock_guard lk{mutex};
        threads.erase(id);
    }

    template <typename Function> void foreach_thread(Function &&f) {
        std::lock_guard lk{mutex};
        for (auto &&[id, thread] : threads) {
            (void)id;
            f(thread);
        }
    }

    void init_global_root_object(ObjectWrapper root) {
        assert(global_root_object_ == nullptr);
        global_root_object_ = root.get_holder();
    }

    ObjectHolder get_global_root_object() const { return global_root_object_; }
};

class ThreadLocalRoot {
private:
    Object *returned_object_ = nullptr;
    Context *top_context_ = nullptr;

public:
    void hold_returned_object(Object *obj) {
        // Consider case obj == returned_object_
        increment_ref_count(obj);
        decrement_ref_count(returned_object_);
        returned_object_ = obj;
    }

    void set_top_context(Context *ctx) { top_context_ = ctx; }

    Context *get_top_context() { return top_context_; }
};
} // namespace ljf

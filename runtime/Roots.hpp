#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>

#include "Object.hpp"

namespace ljf
{

class ThreadLocalRoot;

class GlobalRoot
{
private:
    std::mutex mutex;
    std::unordered_map<std::thread::id, ThreadLocalRoot *> threads;

public:
    void add_thread(std::thread::id id, ThreadLocalRoot *thread)
    {

        std::lock_guard lk{mutex};
        threads[id] = thread;
    }

    void erase_thread(std::thread::id id)
    {
        std::lock_guard lk{mutex};
        threads.erase(id);
    }

    template <typename Function>
    void foreach_thread(Function &&f)
    {
        std::lock_guard lk{mutex};
        for (auto &&[id, thread] : threads)
        {
            (void)id;
            f(thread);
        }
    }
};

using TemporaryStorage = Object;

struct CallStack : Object
{
    explicit CallStack(Environment *env, TemporaryStorage *tmp, CallStack *next)
    {
        ljf::set_object_to_table(this, "env", env);
        ljf::set_object_to_table(this, "tmp", tmp);
        ljf::set_object_to_table(this, "next", next);
    }

    Environment *env()
    {
        return ljf_get(this, "env", ljf::VISIBLE);
    }

    TemporaryStorage *tmp()
    {
        return ljf_get(this, "tmp", ljf::VISIBLE);
    }

    CallStack *next()
    {
        return static_cast<CallStack *>(ljf_get(this, "next", ljf::VISIBLE));
    }
};

class ThreadLocalRoot
{
private:
    Object *returned_object_ = nullptr;
    CallStack *call_stack = nullptr;

public:
    void hold_returned_object(Object *obj)
    {
        // Consider case obj == returned_object_
        increment_ref_count(obj);
        decrement_ref_count(returned_object_); // NOLINT
        returned_object_ = obj;
    }

    void push_call_stack(Environment *env, TemporaryStorage *tmp)
    {
        auto next = call_stack;
        call_stack = new CallStack(env, tmp, next);
    }

    void pop_call_stack()
    {
        auto next = call_stack->next();
        call_stack = next;
    }
};
} // namespace ljf

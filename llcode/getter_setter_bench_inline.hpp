#pragma once

#include <string>
#include <iostream>
#include <forward_list>
#include <list>
#include <thread>
#include <mutex>
#include <algorithm>
#include <stdexcept>

#include <llvm/IR/Function.h>

#include "ljf/runtime.hpp"
#include "../runtime/runtime-internal.hpp"

static size_t allocated_memory_size;
namespace ljf
{
class Object;
using ObjectPtr = Object *;

using FunctionId = LJFFunctionId;

struct TypeObject;
std::shared_ptr<TypeObject> calculate_type(const Object &obj);

enum TableVisiblity
{
    visible,
    hidden
};

class Object
{
public:
    std::recursive_mutex mutex_;
    size_t version_ = 0;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<ObjectPtr> array_table_;
    std::vector<ObjectPtr> array_;
    std::unordered_map<std::string, FunctionId> function_id_table_;
    using native_data_t = std::uint64_t;
    const native_data_t native_data_ = 0;
    ssize_t ref_count_ = 0;

public:
    Object() = default;
    explicit Object(native_data_t data) : native_data_(data) {}
    Object(const Object &) = delete;
    Object(Object &&) = delete;
    Object &operator=(const Object &) = delete;
    Object &operator=(Object &&) = delete;

    Object *get(const std::string &key)
    {
        std::lock_guard lk{mutex_};

        if (!hash_table_.count(key))
        {
            // std::cerr << "ljf get(): no such key: " << key << std::endl;
            return ljf_undefined;
        }

        return array_table_.at(hash_table_.at(key));
    }
    // void set(const std::string &key, ObjectPtr value)
    // {
    //     hash_table_.insert_or_assign(key, value);
    // }

    void set_object_to_table(TableVisiblity visiblity, const char *key, Object *value)
    {

        auto &table = (visiblity == visible) ? hash_table_ : hidden_table_;

        size_t index;
        {
            std::lock_guard lk{mutex_};
            if (!table.count(key))
            {
                index = array_table_new_index();
                table[key] = index;
            }
            else
            {
                index = table.at(key);
            }
        }
        // assert(value != 0);
        // assert(value);

        array_table_set_index(index, value);
    }
    Object *get_object_from_table(TableVisiblity visiblity, const char *key)
    {

        auto &table = (visiblity == visible) ? hash_table_ : hidden_table_;
        std::lock_guard lk{mutex_};

        return array_table_.at(table.at(key));
    }

    FunctionId get_function_id(const std::string &key)
    {
        std::lock_guard lk{mutex_};
        return function_id_table_.at(key);
    }
    void set_function_id(const std::string &key, FunctionId function_id)
    {
        function_id_table_.insert_or_assign(key, function_id);
    }

    Object *&array_table_get_index(uint64_t index)
    {
        return array_table_[index];
    }

    Object *&array_table_set_index(uint64_t index, Object *value)
    {
        increment_ref_count(value);
        decrement_ref_count(array_table_[index]);
        return array_table_[index] = value;
    }

    uint64_t array_table_new_index()
    {
        uint64_t index = array_table_.size();
        array_table_.push_back(ljf_undefined);
        return index;
    }

    void array_table_reserve(uint64_t size)
    {
        throw ljf::runtime_error("unsupported operation");
        // array_table_.reserve(size);
    }

    void array_table_resize(uint64_t size)
    {
        array_table_.resize(size);
    }

    void lock()
    {
        mutex_.lock();
    }

    void unlock()
    {
        mutex_.unlock();
    }

    // array API
    size_t array_size()
    {
        std::lock_guard lk{mutex_};
        return array_.size();
    }
    Object *array_at(uint64_t index)
    {
        std::lock_guard lk{mutex_};
        return array_.at(index);
    }
    void array_set_at(uint64_t index, Object *value)
    {
        Object *old_value;
        {
            std::lock_guard lk{mutex_};
            auto &elem_ref = array_.at(index);
            old_value = elem_ref;
            elem_ref = value;
        }
        assert(value); // DEBUG
        increment_ref_count(value);
        decrement_ref_count(old_value);
    }

    void array_push(Object *value)
    {
        {
            std::lock_guard lk{mutex_};
            array_.push_back(value);
        }
        // assert(value); // DEBUG
        increment_ref_count(value);
    }

    // native data
    uint64_t get_native_data() const
    {
        return native_data_;
    }

    std::shared_ptr<TypeObject> calculate_type()
    {
        std::lock_guard lk{mutex_};
        if (type_object_)
        {
            return type_object_;
        }

        type_object_ = ljf::calculate_type(*this);
        return type_object_;
    }

    ~Object()
    {
        // std::cout << "~Object() " << this << " dump\n";
        // dump();

        for (auto &&obj : array_table_)
        {
            decrement_ref_count(obj);
        }

        for (auto &&obj : array_)
        {
            decrement_ref_count(obj);
        }
    }

    friend void increment_ref_count(Object *obj);
    friend void decrement_ref_count(Object *obj);

    void dump()
    {
        std::cout << "{\n";
        std::cout << "    {\n";
        for (auto &&[key, value] : hash_table_)
        {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    hidden: {\n";
        for (auto &&[key, value] : hidden_table_)
        {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    array table: [";
        for (auto &&v : array_table_)
        {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    array: [";
        for (auto &&v : array_)
        {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    native_data: " << native_data_ << "\n";
        std::cout << "    ref_count: " << ref_count_ << "\n";

        std::cout << "}\n";
    }
};

void increment_ref_count(Object *obj)
{
    if (obj)
    {
        std::lock_guard lk{*obj};
        obj->ref_count_++;
    }
}

void decrement_ref_count(Object *obj)
{
    // std::cerr << "decrement_ref_count() obj: " << obj << std::endl;

    if (!obj)
    {
        return;
    }

    obj->lock();
    assert(obj->ref_count_ > 0);
    obj->ref_count_--;
    if (obj->ref_count_ == 0)
    {
        obj->unlock();
        delete obj;

        // assert(allocated_memory_size >= sizeof(Object));
        // allocated_memory_size -= sizeof(Object);
        return;
    }
    obj->unlock();
}

} // namespace ljf

// -------------------
namespace direct_call_index_access_inline
{
using namespace ljf;
LJFObject *GS_get(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

LJFObject *GS_set(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{

    auto args = new Object;
    ljf::ObjectHolder args_h = args;
    // ljf_internal_resize_object_array_table_size(args, 1);
    args->array_table_resize(1);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{

    auto args = new Object;
    ljf::ObjectHolder args_h = args;
    // ljf_internal_resize_object_array_table_size(args, 2);
    args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    ljf::ObjectHolder v_h = new Object(v);
    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    args->array_table_set_index(1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline

// -------------------
namespace direct_call_index_access_inline_stackallocated
{
using namespace ljf;
LJFObject *GS_get(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

LJFObject *GS_set(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{

    Object args_;
    auto args = &args_;
    // ljf_internal_resize_object_array_table_size(args, 1);
    args->array_table_resize(1);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{

    Object args_;
    auto args = &args_;
    // ljf_internal_resize_object_array_table_size(args, 2);
    args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    ljf::ObjectHolder v_h = new Object(v);
    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    args->array_table_set_index(1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_stackallocated

// -------------------
namespace direct_call_index_access_inline_stackallocated_holding
{
using namespace ljf;
LJFObject *GS_get(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

LJFObject *GS_set(LJFObject *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

Object *volatile hold;

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{

    Object args_;
    auto args = &args_;
    hold = args;
    // ljf_internal_resize_object_array_table_size(args, 1);
    args->array_table_resize(1);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{

    Object args_;
    auto args = &args_;
    hold = args;
    // ljf_internal_resize_object_array_table_size(args, 2);
    args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    ljf_internal_set_object_by_index(args, 0, gs);
    args->array_table_set_index(0, gs);
    ljf::ObjectHolder v_h = new Object(v);
    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    args->array_table_set_index(1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_stackallocated_holding

// -------------------
namespace direct_call_index_access_inline_args_struct_heap_lock
{
using namespace ljf;

struct Args_GS_get
{
    std::mutex mutex_;
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    args->mutex_.lock();
    auto self = args->self;
    args->mutex_.unlock();

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

struct Args_GS_set
{
    std::mutex mutex_;
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args, LJFObject *tmp)
{
    args->mutex_.lock();
    auto self = args->self;
    auto v = args->v;
    args->mutex_.unlock();

    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    // auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{
    auto args = std::make_shared<Args_GS_get>();

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->mutex_.lock();
    args->self = gs;
    args->mutex_.unlock();
    return GS_get(args.get(), nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);
    auto args = std::make_shared<Args_GS_set>();
    args->mutex_.lock();
    args->self = gs;
    args->v = v_h.get();
    args->mutex_.unlock();
    // ljf_internal_resize_object_array_table_size(args, 2);
    // args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    // args->array_table_set_index(1, v_h.get());
    GS_set(args.get(), nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_heap_lock

// -------------------
namespace direct_call_index_access_inline_args_struct_heap_nolock
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    auto self = args->self;

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args, LJFObject *tmp)
{
    auto self = args->self;
    auto v = args->v;

    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    // auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{
    auto args = std::make_shared<Args_GS_get>();

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->self = gs;
    return GS_get(args.get(), nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);
    auto args = std::make_shared<Args_GS_set>();
    args->self = gs;
    args->v = v_h.get();
    // ljf_internal_resize_object_array_table_size(args, 2);
    // args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    // args->array_table_set_index(1, v_h.get());
    GS_set(args.get(), nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_heap_nolock

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_lock
{
using namespace ljf;

struct Args_GS_get
{
    std::mutex mutex_;
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    args->mutex_.lock();
    auto self = args->self;
    args->mutex_.unlock();

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

struct Args_GS_set
{
    std::mutex mutex_;
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args, LJFObject *tmp)
{
    args->mutex_.lock();
    auto self = args->self;
    auto v = args->v;
    args->mutex_.unlock();

    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    // auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->mutex_.lock();
    args->self = gs;
    args->mutex_.unlock();
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);
    Args_GS_set args_;
    auto args = &args_;
    args->mutex_.lock();
    args->self = gs;
    args->v = v_h.get();
    args->mutex_.unlock();
    // ljf_internal_resize_object_array_table_size(args, 2);
    // args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    // args->array_table_set_index(1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_lock

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args, LJFObject *tmp)
{
    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    auto self = args->self;

    // auto v_ = ljf_get_object_from_table(self, "_value");
    // auto v = ljf_internal_get_object_by_index(self, 1);
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args, LJFObject *tmp)
{
    auto self = args->self;
    auto v = args->v;

    // auto self = ljf_get_object_from_table(args, "self");
    // auto self = ljf_internal_get_object_by_index(args, 0);
    // auto self = args->array_table_get_index(0);
    // auto v = ljf_get_object_from_table(args, "v");
    // auto v = ljf_internal_get_object_by_index(args, 1);
    // auto v = args->array_table_get_index(1);
    // assert(v);
    // ljf_set_object_to_table(self, "_value", v);
    // ljf_internal_set_object_by_index(self, 1, v);
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->self = gs;
    return GS_get(args, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v_h.get();
    // ljf_internal_resize_object_array_table_size(args, 2);
    // args->array_table_resize(2);
    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    // ljf_set_object_to_table(args, "v", v_h.get());
    // ljf_internal_set_object_by_index(args, 1, v_h.get());
    // args->array_table_set_index(1, v_h.get());
    GS_set(args, nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock

// -------------------
namespace direct_call_index_access_inline_native_arg_passing
{
using namespace ljf;

LJFObject *GS_get(Object *self, LJFObject *tmp)
{
    auto v = self->array_table_get_index(1);

    // assert(v == v_);
    return v;
}

LJFObject *GS_set(Object *self, Object *v, LJFObject *tmp)
{
    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *env, LJFObject *gs)
{
    return GS_get(gs, nullptr);
}

static void call_GS_set(LJFObject *env, LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);

    GS_set(gs, v_h.get(), nullptr);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(env, G.get()).get());
        call_GS_set(env, S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(env, S.get()).get());
}
} // namespace direct_call_index_access_inline_native_arg_passing

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    auto v = self->array_table_get_index(1);

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(LJFObject *gs, uint64_t v)
{
    ljf::ObjectHolder v_h = new Object(v);
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v_h.get();
    GS_set(args);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(G.get()).get());
        call_GS_set(S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

LJFObject *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    auto v = self->array_table_get_index(1);

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

LJFObject *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    self->array_table_set_index(1, v);
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(LJFObject *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(LJFObject *gs, LJFObject *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = ljf_new_object_with_native_data(0);
    ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(G.get()).get());
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);
    increment_ref_count(v);
    decrement_ref_count(self->array_table_[1]);
    self->array_table_[1] = v;
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    // ljf_internal_resize_object_array_table_size(args, 1);
    // args->array_table_resize(1);

    // ljf_set_object_to_table(args, "self", gs);
    // ljf_internal_set_object_by_index(args, 0, gs);
    // args->array_table_set_index(0, gs);

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = ljf_new_object_with_native_data(0);
    ljf::ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(G.get()).get());
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
    }

    self->array_table_[1] = v;
    return ljf_undefined;
}

static ljf::ObjectHolder call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return reinterpret_cast<ljf::Object *>(GS_get(args));
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}
uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = ljf_new_object_with_native_data(0);
    ljf::ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(G.get()).get());
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(S.get()).get());
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount

// -------------------
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
    }

    self->array_table_[1] = v;
    return ljf_undefined;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = ljf_new_object_with_native_data(0);
    ljf::ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + ljf_get_native_data(call_GS_get(G.get()));
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return ljf_get_native_data(call_GS_get(S.get()));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder

//
//
//
//
namespace newGS_costs
{
using namespace ljf;

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

// Object *GS_set(Args_GS_set *args)
// {
//     auto self = args->self;
//     auto v = args->v;

//     // self->array_table_set_index(1, v);

//     // increment_ref_count(v);
//     if (v)
//     {
//         v->ref_count_++;
//     }
//     // decrement_ref_count(self->array_table_[1]);
//     if (auto old = self->array_table_[1])
//     {
//         // self->array_table_[1]->ref_count_--;
//         old->ref_count_--;
//     }

//     self->array_table_[1] = v;
//     return ljf_undefined;
// }

static ljf::ObjectHolder call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

// static void call_GS_set(Object *gs, Object *v)
// {
//     Args_GS_set args_;
//     auto args = &args_;
//     args->self = gs;
//     args->v = v;
//     GS_set(args);
// }

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    // uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = ljf_new_object_with_native_data(0);
    ljf::ObjectHolder zero_holder = zero;

    return ljf_get_native_data(call_GS_get(S.get()).get());
}
} // namespace newGS_costs

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_small_object
{

struct Object
{
    uint64_t native_data_ = 0;
    std::vector<Object *> array_table_;
    ssize_t ref_count_ = 0;
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
        if (old->ref_count_ == 0)
        {
            delete old;
        }
    }

    self->array_table_[1] = v;
    return nullptr;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

static uint64_t get_native_data(Object *o)
{
    return o->native_data_;
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = new Object{0};
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + get_native_data(call_GS_get(G));
        (void)r;
        call_GS_set(S, zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return get_native_data(call_GS_get(S));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_small_object

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_small_object
{

struct Object
{
    uint64_t native_data_ = 0;
    std::vector<Object *> array_table_;
    ssize_t ref_count_ = 0;
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
        if (old->ref_count_ == 0)
        {
            delete old;
        }
    }

    self->array_table_[1] = v;
    return nullptr;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

static uint64_t get_native_data(Object *o)
{
    return o->native_data_;
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = new Object{0};
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + get_native_data(call_GS_get(G));

        call_GS_set(S, new Object{r});

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return get_native_data(call_GS_get(S));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_small_object

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object
{
struct TypeObject
{
};

struct Object
{
    uint64_t native_data_ = 0;
    std::vector<Object *> array_table_;
    ssize_t ref_count_ = 0;
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<Object *> array_;
    std::unordered_map<std::string, uint64_t> function_id_table_;
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
        if (old->ref_count_ == 0)
        {
            delete old;
        }
    }

    self->array_table_[1] = v;
    return nullptr;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

static uint64_t get_native_data(Object *o)
{
    return o->native_data_;
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = new Object{0};
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + get_native_data(call_GS_get(G));

        call_GS_set(S, new Object{r});

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return get_native_data(call_GS_get(S));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_big_object
{
struct TypeObject
{
};

struct Object
{
    uint64_t native_data_ = 0;
    std::vector<Object *> array_table_;
    ssize_t ref_count_ = 0;
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<Object *> array_;
    std::unordered_map<std::string, uint64_t> function_id_table_;
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
        if (old->ref_count_ == 0)
        {
            delete old;
        }
    }

    self->array_table_[1] = v;
    return nullptr;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

static uint64_t get_native_data(Object *o)
{
    return o->native_data_;
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = new Object{0};
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + get_native_data(call_GS_get(G));
        (void)r;
        call_GS_set(S, zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return get_native_data(call_GS_get(S));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_big_object

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_ljf_object_layout
{
struct TypeObject
{
};

class Object
{
public:
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<Object *> array_table_;
    std::vector<Object *> array_;
    std::unordered_map<std::string, uint64_t> function_id_table_;
    using native_data_t = std::uint64_t;
    const native_data_t native_data_ = 0;
    ssize_t ref_count_ = 0;

public:
    Object(uint64_t v) : native_data_(v) {}
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
        if (old->ref_count_ == 0)
        {
            delete old;
        }
    }

    self->array_table_[1] = v;
    return nullptr;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

static uint64_t get_native_data(Object *o)
{
    return o->native_data_;
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);
    auto zero = new Object{0};
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + get_native_data(call_GS_get(G));
        (void)r;
        call_GS_set(S, zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return get_native_data(call_GS_get(S));
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_ljf_object_layout

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func
{
using namespace ljf;

static ljf::ObjectHolder newGS(LJFObject *env, uint64_t v)
{

    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
    }

    self->array_table_[1] = v;
    return ljf_undefined;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = new Object(0);
    ljf::ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + (call_GS_get(G.get()))->native_data_;
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return (call_GS_get(S.get()))->native_data_;
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_neverObjectHolder
{
using namespace ljf;

static Object *newGS(LJFObject *env, uint64_t v)
{

    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
    }

    self->array_table_[1] = v;
    return ljf_undefined;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    Object *G = newGS(env, 0);
    Object *S = newGS(env, 0);
    auto zero = new Object(0);
    zero->ref_count_++;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + call_GS_get(G)->native_data_;
        (void)r;
        call_GS_set(S, zero);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return call_GS_get(S)->native_data_;
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_neverObjectHolder

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_efficient_ObjectHolder_swap
{
using namespace ljf;

static ljf::ObjectHolder newGS(LJFObject *env, uint64_t v)
{

    auto v_ = new Object{v};
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = v_;
    v_->ref_count_++;

    return r;
}

struct Args_GS_get
{
    Object *self;
};

Object *GS_get(Args_GS_get *args)
{
    auto self = args->self;
    // auto v = self->array_table_get_index(1);
    auto v = self->array_table_[1];

    return v;
}

struct Args_GS_set
{
    Object *self;
    Object *v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    // self->array_table_set_index(1, v);

    // increment_ref_count(v);
    if (v)
    {
        v->ref_count_++;
    }
    // decrement_ref_count(self->array_table_[1]);
    if (auto old = self->array_table_[1])
    {
        // self->array_table_[1]->ref_count_--;
        old->ref_count_--;
    }

    self->array_table_[1] = v;
    return ljf_undefined;
}

static Object *call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, Object *v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);
    auto zero = new Object(0);
    ljf::ObjectHolder zero_holder = zero;

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + (call_GS_get(G.get()))->native_data_;
        (void)r;
        call_GS_set(S.get(), zero);

        auto tmp = std::move(G);
        G = std::move(S);
        S = std::move(tmp);
    }

    return (call_GS_get(S.get()))->native_data_;
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_same_V_cheating_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_efficient_ObjectHolder_swap

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object_primitive_optimization
{
struct TypeObject
{
};

struct Object
{
    uint64_t native_data_ = 0;
    std::vector<Object *> array_table_;
    ssize_t ref_count_ = 0;
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, size_t> hash_table_;
    std::unordered_map<std::string, size_t> hidden_table_;
    std::vector<Object *> array_;
    std::unordered_map<std::string, uint64_t> function_id_table_;
};

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = reinterpret_cast<Object *>(v);
    return r;
}

struct Args_GS_get
{
    Object *self;
};

uint64_t GS_get(Args_GS_get *args)
{
    auto self = args->self;
    static_assert(sizeof(Object *) == sizeof(uint64_t));
    auto v = reinterpret_cast<uint64_t>(self->array_table_[1]);

    return v;
}

struct Args_GS_set
{
    Object *self;
    uint64_t v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    static_assert(sizeof(Object *) == sizeof(uint64_t));
    self->array_table_[1] = reinterpret_cast<Object *>(v);
    return nullptr;
}

static uint64_t call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, uint64_t v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    auto G = newGS(env, 0);
    auto S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + call_GS_get(G);
        call_GS_set(S, r);

        auto tmp = G;
        G = S;
        S = tmp;
    }

    return call_GS_get(S);
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_big_object_primitive_optimization

// -------------------
//
//
//
//
namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_primitive_optimization
{
using namespace ljf;

static Object *newGS(LJFObject *env, uint64_t v)
{
    auto r = new Object{v};
    r->array_table_.resize(2);
    r->array_table_[1] = reinterpret_cast<Object *>(v);
    return r;
}

struct Args_GS_get
{
    Object *self;
};

uint64_t GS_get(Args_GS_get *args)
{
    auto self = args->self;
    static_assert(sizeof(Object *) == sizeof(uint64_t));
    auto v = reinterpret_cast<uint64_t>(self->array_table_[1]);

    return v;
}

struct Args_GS_set
{
    Object *self;
    uint64_t v;
};

Object *GS_set(Args_GS_set *args)
{
    auto self = args->self;
    auto v = args->v;

    static_assert(sizeof(Object *) == sizeof(uint64_t));
    self->array_table_[1] = reinterpret_cast<Object *>(v);
    return nullptr;
}

static uint64_t call_GS_get(Object *gs)
{
    Args_GS_get args_;
    auto args = &args_;

    args->self = gs;
    return GS_get(args);
}

static void call_GS_set(Object *gs, uint64_t v)
{
    Args_GS_set args_;
    auto args = &args_;
    args->self = gs;
    args->v = v;
    GS_set(args);
}

uint64_t getter_setter_bench_ljf(LJFObject *env, uint64_t n)
{
    uint64_t k = 1;

    ljf::ObjectHolder G = newGS(env, 0);
    ljf::ObjectHolder S = newGS(env, 0);

    for (size_t i = 0; i < n; i++)
    {
        auto r = k + call_GS_get(G.get());
        call_GS_set(S.get(), r);

        auto tmp = G;
        G = S;
        S = tmp;
    }
    auto ret = call_GS_get(S.get());

    // put nullptr to array_table_[1] otherwise ~Object() destruct invalid ptr of array_table_[1]
    call_GS_set(S.get(), 0);
    call_GS_set(G.get(), 0);

    return ret;
}
} // namespace direct_call_index_access_inline_args_struct_stack_nolock_refactor_unwrap_object_method_nolock_refcount_noObjectHolder_no_ljf_func_primitive_optimization

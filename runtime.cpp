
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
#include "ljf/initmsg.hpp"

namespace ljf {
InitMsg im {"ljf runtime loading"};
DoneMsg dm {"ljf runtime unloading"};

class Object;
using ObjectPtr = Object*;

using FunctionId = LJFFunctionId;

struct TypeObject;

} // namespace ljf

namespace std
{
    template<>
    struct hash<ljf::TypeObject>
    {
        size_t operator()(const ljf::TypeObject& typ) const noexcept {
            return 0;
        }
    };
    
} // namespace std


namespace ljf {

std::shared_ptr<TypeObject> calculate_type(const Object& obj);

class Object
{
public:
    std::mutex mutex_;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<std::string, ObjectPtr> hash_table_;
    std::unordered_map<std::string, ObjectPtr> hidden_table_;
    std::vector<ObjectPtr> array_;
    std::unordered_map<std::string, FunctionId> function_id_table_;
    using native_data_t = std::uint64_t;
    const native_data_t native_data_ = 0;
    int ref_count_ = 0;
public:
    Object() = default;
    explicit Object(native_data_t data) : native_data_(data) {}
    Object(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(const Object&) = delete;
    Object& operator=(Object&&) = delete;

    ObjectPtr get(const std::string& key) const {
        return hash_table_.at(key);
    }
    void set(const std::string& key, ObjectPtr value) {
        hash_table_.insert_or_assign(key, value);
    }

    FunctionId get_function_id(const std::string& key) {
        return function_id_table_.at(key);
    }
    void set_function_id(const std::string& key, FunctionId function_id) {
        function_id_table_.insert_or_assign(key, function_id);
    }

    std::shared_ptr<TypeObject> calculate_type() {
        std::lock_guard lk {mutex_};
        if (type_object_) {
            return type_object_;
        }

        type_object_ = ljf::calculate_type(*this);
        return type_object_;
    }
};

using TemporaryStorage = Object;

struct TypeObject
{
    /* data */
    bool operator==(const TypeObject& other) const {
        return true;
    };
};
std::shared_ptr<TypeObject> calculate_type(const Object& obj) {
    return std::make_shared<TypeObject>();
}


using FunctionPtr = Object*(*)(Environment*, TemporaryStorage*);

struct FunctionData
{
    // naive means 'not optimized'
    const llvm::Function* naive_llvm_function;

    FunctionPtr naive_function = nullptr;
    
    struct DataForArgType
    {
        std::size_t called_count = 0;

        // specialized function for argument types
        llvm::Function* specialized_function = nullptr;
    };

    std::unordered_map<TypeObject, DataForArgType> data_for_arg_type;
};


// class Root : public Object
// {
// private:
//     std::mutex mutex;
//     std::unordered_map<std::thread::id, Object*> threads;
// public:
//     void set_thread(std::thread::id id, Object* obj) {
//         std::lock_guard lk {mutex};
//         threads[id] = obj;
//     }

//     void erase_thread(std::thread::id id) {
//         std::lock_guard lk {mutex};
//         threads.erase(id);
//     }

//     std::vector<Object*> get_all_threads() {
//         std::lock_guard lk {mutex};
//         std::vector<Object*> ret;
//         for (auto&& [id, obj] : threads)
//         {
//             ret.push_back(obj);
//         }
//         return ret;
//     }
// };

struct ThreadLocalRoot;

class GlobalRoot
{
private:
    std::mutex mutex;
    std::unordered_map<std::thread::id, ThreadLocalRoot*> threads;
public:
    void set_thread(std::thread::id id, ThreadLocalRoot* thread) {

        std::lock_guard lk {mutex};
        threads[id] = thread;
    }

    void erase_thread(std::thread::id id) {
        std::lock_guard lk {mutex};
        threads.erase(id);
    }

    template<typename Function>
    void foreach_thread(Function&& f) {
        std::lock_guard lk {mutex};
        for (auto&& [id, thread] : threads)
        {
            (void) id;
            f(thread);
        }
    }
};

struct CallStack : Object {
    explicit
    CallStack(Environment* env, TemporaryStorage* tmp, CallStack* next) {
        ljf_set_object_to_table(this, "env", env);
        ljf_set_object_to_table(this, "tmp", tmp);
        ljf_set_object_to_table(this, "next", next);
    }

    Environment* env() {
        return ljf_get_object_from_table(this, "env");
    }

    TemporaryStorage* tmp() {
        return ljf_get_object_from_table(this, "tmp");
    }

    CallStack* next() {
        return static_cast<CallStack*>(ljf_get_object_from_table(this, "next"));
    }


};

struct ThreadLocalRoot {
    Object* returned_object_ = nullptr;
    CallStack* call_stack = nullptr;

    void hold_returned_object(Object* obj) {
        returned_object_ = obj;
    }

    void push_call_stack(Environment* env, TemporaryStorage* tmp) {
        auto next = call_stack;
        call_stack = new CallStack(env, tmp, next);
    }

    void pop_call_stack() {
        auto next = call_stack->next();
        call_stack = next;
    }
};


// class CallFrameData {
//     Object obj;
//     Root& root_;

// public:
//     explicit
//     CallFrameData(Root& root) : root_(root) {
//         auto thread_id = std::this_thread::get_id();
//         root.set_thread(thread_id, &obj);
//     }

//     ~CallFrameData() {
//         auto thread_id = std::this_thread::get_id();
//         root_.erase_thread(thread_id);
//     }

//     void hold_returned_object(Object* obj) {

//     }
// };

class FunctionTable
{
private:
    std::mutex mutex_;
    std::size_t size_;
    std::unordered_map<FunctionId, FunctionData> function_table_;

    FunctionId add(const FunctionData& f) {
        std::lock_guard lk {mutex_};
        const auto id = size_;
        size_++;
        function_table_.insert_or_assign(id, f);
        return id;
    }
public:

    FunctionId add_native(FunctionPtr fn) {
        return add({nullptr, fn});
    }

    FunctionId add_llvm(llvm::Function* fn) {
        return add({fn, nullptr});
    }

    void set_native(FunctionId id, FunctionPtr fn) {
        std::lock_guard lk {mutex_};
        try
        {
            function_table_.at(id).naive_function = fn;
        }
        catch(const std::out_of_range&)
        {
            throw ljf::runtime_error("error: set function to invalid function id");
        }
        
    }

    FunctionData& get(FunctionId id) {
        try
        {
            return function_table_.at(id);
        }
        catch(std::out_of_range)
        {
            throw ljf::runtime_error("no such function id");
        }
        
    }
};

namespace {
    FunctionTable function_table;
    GlobalRoot global_root;
    thread_local ThreadLocalRoot* thread_local_root = []() {
        auto th = new ThreadLocalRoot;
        global_root.set_thread(std::this_thread::get_id(), th);
        return th;
    }();
    struct ThreadLocalRootEraser {
        ~ThreadLocalRootEraser() {
            global_root.erase_thread(std::this_thread::get_id());
        }
    };
    thread_local ThreadLocalRootEraser eraser;

}

FunctionId register_llvm_function(llvm::Function* f) {
    return function_table.add_llvm(f);
}

void check_not_null(const Object* obj) {
    if (!obj) {
        throw runtime_error("null pointer exception");
    }
}

} // namespace ljf

using namespace ljf;

namespace ljf::internal
{
    Environment* create_environment() {
        auto env = new Environment;
        auto env_maps = ljf_new_object();
        ljf_set_object_to_hidden_table(env, "ljf.env.maps", env_maps);

        // set maps[0]
        env_maps->array_.push_back(ljf_new_object());

        return env;
    }
} // namespace ljf::internal




extern "C"
void ljf_internal_set_native_function(FunctionId id, FunctionPtr fn) {
    function_table.set_native(id, fn);
}

namespace {
    

    Environment*
    create_callee_environment(Environment* parent, Object* arg) {
        auto callee_env = internal::create_environment();
        auto callee_env_maps = ljf_get_object_from_hidden_table(callee_env, "ljf.env.maps");

        // set maps[1]
        callee_env_maps->array_.push_back(arg);

        if (!parent)
        {
            return callee_env;
        }
        
        auto parent_env_maps = ljf_get_object_from_hidden_table(parent, "ljf.env.maps");

        if (!parent_env_maps)
        {
            return callee_env;
        }

        // set maps[2:]
        auto begin = parent_env_maps->array_.begin();
        auto end = parent_env_maps->array_.end();
        std::copy(begin, end, std::back_inserter(callee_env_maps->array_));

        return callee_env;
    }
} // namespace

Object* ljf_get_object_from_table(Object* obj, const char* key) {
    check_not_null(obj);

    return obj->hash_table_[key];
}

void ljf_set_object_to_table(Object* obj, const char* key, Object* value) {
    check_not_null(obj);
    
    obj->hash_table_.insert_or_assign(key, value);
    if (value)
    {
        value->ref_count_++;
    }
}

Object* ljf_get_object_from_hidden_table(Object* obj, const char* key) {
    check_not_null(obj);
    
    return obj->hidden_table_[key];
}

void ljf_set_object_to_hidden_table(Object* obj, const char* key, Object* value) {
    check_not_null(obj);
    
    obj->hidden_table_.insert_or_assign(key, value);
    value->ref_count_++;
}

void ljf_set_object_to_array(Object* obj, size_t index, Object* value) {
    check_not_null(obj);
    
    obj->array_.at(index) = value;
    value->ref_count_++;
}

FunctionId ljf_get_function_id_from_function_table(const Object* obj, const char* key) {
    check_not_null(obj);
    
    try {
        return obj->function_id_table_.at(key);
    } catch (std::out_of_range) {
        throw ljf::runtime_error("function table: no such key: " + std::string(key));
    }
}

void ljf_set_function_id_to_function_table(Object* obj, const char* key, FunctionId function_id) {
    check_not_null(obj);
    
    obj->function_id_table_.insert_or_assign(key, function_id);
}

Object* ljf_call_function(FunctionId function_id, Environment* env, Object* arg) {
    auto& func_data = function_table.get(function_id);
    auto callee_env = create_callee_environment(env, arg);
    auto arg_type = callee_env->calculate_type();

    auto& data_for_arg_type = func_data.data_for_arg_type[*arg_type];
    data_for_arg_type.called_count++;

    FunctionPtr func_ptr = func_data.naive_function;
    TemporaryStorage tmp;
    return func_ptr(callee_env, &tmp);
}

Object* ljf_new_object_with_native_data(uint64_t data) {
    Object* obj = new Object(data);
    thread_local_root->hold_returned_object(obj);
    return obj;
}

Object* ljf_new_object() {
    return ljf_new_object_with_native_data(0);
}

uint64_t ljf_get_native_data(const Object* obj) {
    check_not_null(obj);
    
    return obj->native_data_;
}

Object* ljf_get_object_from_environment(Environment* env, const char* key) {
    check_not_null(env);

    auto maps = ljf_get_object_from_hidden_table(env, "ljf.env.maps");

    if (! maps) {
        throw ljf::runtime_error("ljf_get_object_from_environment: not a Environment");
    }

    std::lock_guard lk {maps->mutex_};
    for (auto obj : maps->array_) {
        auto value = ljf_get_object_from_table(obj, key);
        if (value) {
            return value;
        }
    }

    return ljf_undefined;
}

void ljf_set_object_to_environment(Environment* env, const char* key, Object* value) {
    check_not_null(env);

    auto maps = ljf_get_object_from_hidden_table(env, "ljf.env.maps");

    if (! maps) {
        throw ljf::runtime_error("ljf_get_object_from_environment: not a Environment");
    }

    std::lock_guard lk {maps->mutex_};

    auto map0 = maps->array_.at(0);
    ljf_set_object_to_table(map0, key, value);
}

FunctionId ljf_register_native_function(Object* (*fn)(Environment*, TemporaryStorage*)) {
    return function_table.add_native(fn);
}

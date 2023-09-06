#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ljf/runtime.hpp"

#include "AttributeTraits.hpp"

namespace ljf {
class Object;
using ObjectPtr = Object *;

class TypeObject;
struct TypeCalcData {
    std::unordered_set<Object *> objects_in_calculation;
};

std::shared_ptr<TypeObject> calculate_type(Object &obj);
std::shared_ptr<TypeObject> calculate_type(Object &obj, TypeCalcData &);

class Object {
private:
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

    void swap(Object &other) {
        std::scoped_lock lk{*this, other};

        type_object_.swap(other.type_object_);
        hash_table_.swap(other.hash_table_);
        hidden_table_.swap(other.hidden_table_);
        array_table_.swap(other.array_table_);
        array_.swap(other.array_);
        function_id_table_.swap(other.function_id_table_);

        ++version_;
        ++other.version_;
    }

    Object *get(const char *key, LJFAttribute attr) {
        auto &table = AttributeTraits::is_visible(attr) ? hash_table_ : hidden_table_;
        std::lock_guard lk{mutex_};

        return array_table_.at(table.at(key));
    }

    void set(const char *key, Object *value, LJFAttribute attr) {

        auto &table =
            AttributeTraits::is_visible(attr) ? hash_table_ : hidden_table_;

        size_t index;
        {
            std::lock_guard lk{mutex_};
            if (!table.count(key)) {
                index = array_table_new_index();
                table[key] = index;
            } else {
                index = table.at(key);
            }
        }
        // assert(value != 0);
        // assert(value);

        array_table_set_index(index, value);
        ++version_;
    }

    FunctionId get_function_id(const std::string &key) {
        std::lock_guard lk{mutex_};
        return function_id_table_.at(key);
    }
    void set_function_id(const std::string &key, FunctionId function_id) {
        function_id_table_.insert_or_assign(key, function_id);
    }

    Object *&array_table_get_index(uint64_t index) {
        return array_table_[index];
    }

    Object *&array_table_set_index(uint64_t index, Object *value) {
        increment_ref_count(value);
        decrement_ref_count(array_table_[index]);
        return array_table_[index] = value;
    }

    uint64_t array_table_new_index() {
        uint64_t index = array_table_.size();
        array_table_.push_back(ljf_undefined);
        return index;
    }

    void array_table_reserve(uint64_t size) {
        throw ljf::runtime_error("unsupported operation");
        // array_table_.reserve(size);
    }

    void array_table_resize(uint64_t size) { array_table_.resize(size); }

    void lock() { mutex_.lock(); }

    bool try_lock() { return mutex_.try_lock(); }

    void unlock() { mutex_.unlock(); }

    // array API
    size_t array_size() {
        std::lock_guard lk{mutex_};
        return array_.size();
    }
    Object *array_at(uint64_t index) {
        std::lock_guard lk{mutex_};
        return array_.at(index);
    }
    void array_set_at(uint64_t index, Object *value) {
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

    void array_push(Object *value) {
        {
            std::lock_guard lk{mutex_};
            array_.push_back(value);
            ++version_;
        }
        // assert(value); // DEBUG
        increment_ref_count(value);
    }

    // native data
    uint64_t get_native_data() const { return native_data_; }

    std::shared_ptr<TypeObject> calculate_type() {
        std::lock_guard lk{mutex_};
        if (type_object_) {
            return type_object_;
        }

        type_object_ = ljf::calculate_type(*this);
        return type_object_;
    }

    std::shared_ptr<TypeObject> calculate_type(TypeCalcData &type_calc_data) {
        std::lock_guard lk{mutex_};
        if (type_object_) {
            return type_object_;
        }

        type_object_ = ljf::calculate_type(*this, type_calc_data);
        return type_object_;
    }

    ~Object() {
        // std::cout << "~Object() " << this << "\n";
        // std::cout << " dump\n";
        // dump();

        for (auto &&obj : array_table_) {
            decrement_ref_count(obj);
        }

        for (auto &&obj : array_) {
            decrement_ref_count(obj);
        }
    }

    class TableIterator;
    TableIterator iter_hash_table();
    TableIterator iter_hidden_table();

    class ArrayIterator;
    ArrayIterator iter_array();

    friend void increment_ref_count(Object *obj);
    friend void decrement_ref_count(Object *obj);

    void dump() {
        std::cout << this << ":\n";
        std::cout << "{\n";
        std::cout << "    {\n";
        for (auto &&[key, value] : hash_table_) {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    hidden: {\n";
        for (auto &&[key, value] : hidden_table_) {
            std::cout << "        " << key << ": " << value << "\n";
        }
        std::cout << "    }\n";

        std::cout << "    array table: [";
        for (auto &&v : array_table_) {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    array: [";
        for (auto &&v : array_) {
            std::cout << v << ", ";
        }
        std::cout << "]\n";

        std::cout << "    native_data: " << native_data_ << "\n";
        std::cout << "    ref_count: " << ref_count_ << "\n";

        std::cout << "}\n";
    }
};

inline void set_object_to_table(Object *obj, const char *key, Object *value) {
    ::ljf_set(obj, key, value, LJFAttribute::VISIBLE);
}

inline Object *get_object_from_hidden_table(Object *obj, const char *key) {
    return ::ljf_get(obj, key, LJFAttribute::HIDDEN);
}

inline void set_object_to_hidden_table(Object *obj, const char *key,
                                       Object *value) {
    ::ljf_set(obj, key, value, LJFAttribute::HIDDEN);
}

inline void increment_ref_count(Object *obj) {
    if (obj) {
        std::lock_guard lk{*obj};
        obj->ref_count_++;
    }
}

inline void decrement_ref_count(Object *obj) {
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
} // namespace ljf

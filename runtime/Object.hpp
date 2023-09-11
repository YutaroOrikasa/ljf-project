#pragma once

#include <assert.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ljf/runtime.hpp"

#include "AttributeTraits.hpp"
#include "ObjectHolder.hpp"
#include "ljf/internal/object-fwd.hpp"

namespace ljf {
class Object;

class Key {
private:
    LJFAttribute attr_;
    const void *key_;

    LJFAttribute mask_attr() const {
        return AttributeTraits::mask(attr_, LJFAttribute::KEY_TYPE_MASK);
    }

public:
    Key(LJFAttribute attr, const void *key) : attr_(attr), key_(key) {}

    Key() = default;
    Key(const Key &) = default;
    Key(Key &&) = default;
    Key &operator=(const Key &) = default;
    Key &operator=(Key &&) = default;

    bool is_c_str_key() const { return mask_attr() == LJFAttribute::C_STR_KEY; }
    bool is_object_key() const {
        return mask_attr() == LJFAttribute::OBJECT_KEY;
    }

    const char *get_key_as_c_str() const {
        assert(is_c_str_key());
        return static_cast<const char *>(key_);
    }
    const Object *get_key_as_object() const {
        assert(is_object_key());
        return static_cast<const Object *>(key_);
    }

    size_t hash_code() const {
        if (is_c_str_key()) {
            return std::hash<std::string_view>()(
                std::string_view(get_key_as_c_str()));
        } else {
            throw "not implemented";
        }
    }

    bool operator==(const Key &other) const {
        return (mask_attr() == other.mask_attr()) && key_ == other.key_;
    }
};
} // namespace ljf

template <> struct std::hash<ljf::Key> {
    size_t operator()(const ljf::Key &key) const { return key.hash_code(); }
};

namespace ljf {
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
    std::unordered_map<Key, size_t> hash_table_;
    std::unordered_map<Key, size_t> hidden_table_;
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

    IncrementedObjectPtrOrNativeValue get(const void *key, LJFAttribute attr) {
        Key key_obj{attr, key};
        auto &table =
            AttributeTraits::is_visible(attr) ? hash_table_ : hidden_table_;

        if (AttributeTraits::mask(attr, LJFAttribute::DATA_TYPE_MASK) ==
            LJFAttribute::OBJECT) {
            std::lock_guard lk{mutex_};
            //  We have to increment returned object because:
            //      returned object will released if other thread decrement refcount
            auto ret = array_table_.at(table.at(key_obj));
            increment_ref_count(ret);
            return static_cast<IncrementedObjectPtrOrNativeValue>(
                reinterpret_cast<uint64_t>(ret));

        } else {
            std::lock_guard lk{mutex_};
            auto ret = array_table_.at(table.at(key_obj));
            return static_cast<IncrementedObjectPtrOrNativeValue>(
                reinterpret_cast<uint64_t>(ret));
        }
    }

    void set(const void *key, Object *value, LJFAttribute attr) {

        Key key_obj{attr, key};
        auto &table =
            AttributeTraits::is_visible(attr) ? hash_table_ : hidden_table_;

        size_t index;
        {
            std::lock_guard lk{mutex_};
            if (!table.count(key_obj)) {
                index = array_table_new_index();
                table[key_obj] = index;
            } else {
                index = table.at(key_obj);
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
        array_table_.push_back(nullptr);
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
    ObjectHolder array_at(uint64_t index) {
        std::lock_guard lk{mutex_};
        return ObjectHolder(array_.at(index));
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
};

inline void set_object_to_table(Object *obj, const char *key, Object *value) {
    obj->set(const_cast<char *>(key), value,
             AttributeTraits::or_attr(LJFAttribute::VISIBLE,
                                      LJFAttribute::C_STR_KEY));
}

inline ObjectHolder get_object_from_hidden_table(Object *obj, const char *key) {
    return obj->get(const_cast<char *>(key),
                    AttributeTraits::or_attr(LJFAttribute::HIDDEN,
                                             LJFAttribute::C_STR_KEY));
}

inline void set_object_to_hidden_table(Object *obj, const char *key,
                                       Object *value) {
    obj->set(const_cast<char *>(key), value,
             AttributeTraits::or_attr(LJFAttribute::HIDDEN,
                                      LJFAttribute::C_STR_KEY));
}
void increment_ref_count(Object *obj);
void decrement_ref_count(Object *obj);
} // namespace ljf

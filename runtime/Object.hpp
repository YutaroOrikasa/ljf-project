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
#include "runtime-internal.hpp"

namespace ljf {
class Object;

class Key {
private:
    LJFAttribute attr_;
    const void *key_;

    LJFAttribute mask_key_attr() const {
        return AttributeTraits::mask(attr_, LJF_ATTR_KEY_ATTR_MASK);
    }

    LJFAttribute mask_key_type_attr() const {
        return AttributeTraits::mask(attr_, LJF_ATTR_KEY_TYPE_MASK);
    }

public:
    Key(LJFAttribute attr, const void *key) : attr_(attr), key_(key) {}

    Key() = default;
    Key(const Key &) = default;
    Key(Key &&) = default;
    Key &operator=(const Key &) = default;
    Key &operator=(Key &&) = default;

    bool is_c_str_key() const {
        return mask_key_type_attr() == LJF_ATTR_C_STR_KEY;
    }
    bool is_object_key() const {
        return mask_key_type_attr() == LJF_ATTR_OBJECT_KEY;
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

        if (mask_key_attr() != other.mask_key_attr()) {
            return false;
        }
        if (is_c_str_key()) {
            return std::string_view(get_key_as_c_str()) ==
                   std::string_view(other.get_key_as_c_str());
        } else {
            throw "not implemented";
        }
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
public:
    class ValueType {
        LJFAttribute attr_;
        ObjectPtrOrNativeValue value_;

    public:
        ValueType()
            : ValueType(LJF_ATTR_DEFAULT,
                        cast_object_to_ObjectPtrOrNativeValue(
                            // ljf_internal_nullptr means uninitialized in this
                            // context eg, sparse array.
                            internal::ljf_internal_nullptr)) {}

        ValueType(LJFAttribute attr, ObjectPtrOrNativeValue value)
            : attr_(AttributeTraits::mask(attr, LJF_ATTR_VALUE_ATTR_MASK)),
              value_(value) {}

        LJFAttribute value_attr() const { return attr_; }
        ObjectPtrOrNativeValue object_ptr_or_native() const { return value_; }

        bool is_object() const {
            return AttributeTraits::mask(attr_, LJF_ATTR_DATA_TYPE_MASK) ==
                   LJF_ATTR_BOXED_OBJECT;
        }

        Object *as_object() const {
            assert(is_object());
            return reinterpret_cast<Object *>(value_);
        }

        uint64_t as_native_value() const {
            assert(!is_object());
            return static_cast<uint64_t>(value_);
        }
    };

private:
    std::recursive_mutex mutex_;
    size_t version_ = 0;
    std::shared_ptr<TypeObject> type_object_;
    std::unordered_map<Key, size_t> hash_table_;
    std::vector<ValueType> array_table_;
    std::vector<ObjectPtr> array_;
    std::unordered_map<std::string, FunctionId> function_id_table_;
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
        array_table_.swap(other.array_table_);
        array_.swap(other.array_);
        function_id_table_.swap(other.function_id_table_);

        ++version_;
        ++other.version_;
    }

    IncrementedObjectPtrOrNativeValue get(const void *key, LJFAttribute attr) {
        Key key_obj{attr, key};

        {
            std::lock_guard lk{mutex_};
            auto ret = array_table_.at(hash_table_.at(key_obj));
            //  We have to increment returned object because:
            //      returned object will released if other thread decrement
            //      refcount
            increment_ref_count_if_object(ret);
            return static_cast<IncrementedObjectPtrOrNativeValue>(
                ret.object_ptr_or_native());
        }
    }

    void set(const void *key, ObjectPtrOrNativeValue value, LJFAttribute attr) {

        Key key_obj{attr, key};

        size_t index;
        {
            std::lock_guard lk{mutex_};
            if (!hash_table_.count(key_obj)) {
                index = array_table_new_index();
                hash_table_[key_obj] = index;
            } else {
                index = hash_table_.at(key_obj);
            }
            ValueType v{attr, value};
            increment_ref_count_if_object(v);
            decrement_ref_count_if_object(array_table_[index]);
            array_table_[index] = v;
            ++version_;
        }
    }

    void set(const void *key, Object *value, LJFAttribute attr) {
        set(key, cast_object_to_ObjectPtrOrNativeValue(value), attr);
    }

    static void increment_ref_count_if_object(const ValueType &value) {

        if (value.is_object()) {
            auto value_obj_ptr = value.as_object();
            increment_ref_count(value_obj_ptr);
        }
    }

    static void decrement_ref_count_if_object(const ValueType &value) {
        if (value.is_object()) {
            auto value_obj_ptr = value.as_object();
            decrement_ref_count(value_obj_ptr);
        }
    }

    FunctionId get_function_id(const std::string &key) {
        std::lock_guard lk{mutex_};
        return function_id_table_.at(key);
    }
    void set_function_id(const std::string &key, FunctionId function_id) {
        function_id_table_.insert_or_assign(key, function_id);
    }

    // disable unsafe api
    // Object *&array_table_get_index(uint64_t index) {
    //     return array_table_[index];
    // }

    // Object *&array_table_set_index(uint64_t index, Object *value) {
    //     increment_ref_count(value);
    //     decrement_ref_count(array_table_[index]);
    //     return array_table_[index] = value;
    // }

    uint64_t array_table_new_index() {
        uint64_t index = array_table_.size();
        array_table_.push_back(ValueType{LJF_ATTR_DEFAULT,
                                         // ljf_internal_nullptr means variable
                                         // will be initialized in future.
                                         cast_object_to_ObjectPtrOrNativeValue(
                                             internal::ljf_internal_nullptr)});
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
            decrement_ref_count_if_object(obj);
        }

        for (auto &&obj : array_) {
            decrement_ref_count(obj);
        }
    }

    class TableIterator;

    class TableRange;
    TableRange iter_hash_table();
    class ArrayIterator;
    ArrayIterator iter_array();

    friend void increment_ref_count(Object *obj);
    friend void decrement_ref_count(Object *obj);
};

inline void set_object_to_table(Object *obj, const char *key, Object *value) {
    obj->set(const_cast<char *>(key),
             cast_object_to_ObjectPtrOrNativeValue(value),
             AttributeTraits::or_attr(LJF_ATTR_VISIBLE, LJF_ATTR_C_STR_KEY));
}

inline ObjectHolder get_object_from_hidden_table(Object *obj, const char *key) {
    return obj->get(
        const_cast<char *>(key),
        AttributeTraits::or_attr(LJF_ATTR_HIDDEN, LJF_ATTR_C_STR_KEY));
}

inline void set_object_to_hidden_table(Object *obj, const char *key,
                                       Object *value) {
    obj->set(const_cast<char *>(key),
             cast_object_to_ObjectPtrOrNativeValue(value),
             AttributeTraits::or_attr(LJF_ATTR_HIDDEN, LJF_ATTR_C_STR_KEY));
}
void increment_ref_count(Object *obj);
void decrement_ref_count(Object *obj);
} // namespace ljf

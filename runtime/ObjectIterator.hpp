#pragma once

#include "Object.hpp"
#include "runtime-internal.hpp"
#include "gtest/gtest.h"

namespace ljf {

class BrokenIteratorError : public runtime_error {
public:
    using runtime_error::runtime_error;
};

class Object::TableIterator {
private:
    TableVisiblity visiblity_;
    size_t version_;
    ObjectHolder obj_;
    std::unordered_map<Key, size_t>::iterator map_iter_;
    std::unordered_map<Key, size_t>::iterator map_iter_end_;

    // TODO remove this when compile ok
    // /// Caller must hold lock of obj.
    // explicit TableIterator(
    //     ObjectHolder obj,
    //     const std::unordered_map<Key, size_t>::iterator &map_iter,
    //     const std::unordered_map<Key, size_t>::iterator &map_iter_end)
    //     : obj_(obj) {
    //     version_ = obj->version_;
    //     map_iter_ = map_iter;
    //     map_iter_end_ = map_iter_end;
    // }

    /// - check object version
    /// - check iterator not ended
    /// If checking is not ok, it will throw exception.
    /// Caller must hold lock of obj_.
    void check() const {
        if (version_ != obj_->version_) {
            throw BrokenIteratorError("Object changed while iteration");
        }

        if (is_end()) {
            throw std::out_of_range("iterator ended");
        }
    }

public:
    struct KeyValue {
        Key key;
        ObjectHolder value;
    };

    explicit TableIterator(ObjectHolder obj, TableVisiblity visiblity)
        : visiblity_(visiblity), obj_(obj) {
        std::lock_guard lk{*obj_};
        version_ = obj->version_;

        map_iter_ = obj->hash_table_.begin();
        map_iter_end_ = obj->hash_table_.end();
    }

    explicit TableIterator(
        ObjectHolder obj,
        std::unordered_map<Key, size_t>::iterator map_iter_end,
        TableVisiblity visiblity)
        : obj_(obj), map_iter_(map_iter_end), map_iter_end_(map_iter_end) {
        std::lock_guard lk{*obj_};
        version_ = obj->version_;

    }

    /// @brief return current pointing KeyValue and go next
    /// This function has same semantics as *(iter++)
    /// @return current pointing KeyValue
    KeyValue next() {
        std::lock_guard lk{*obj_};
        check();

        auto &key = map_iter_->first;
        Object *value = obj_->array_table_.at(map_iter_->second).as_object();

        ++map_iter_;
        return KeyValue{key, value};
    }

    TableIterator operator++() {
        ++map_iter_;
        return *this;
    }

    KeyValue operator*() {
        std::lock_guard lk{*obj_};
        check();

        auto &key = map_iter_->first;
        Object *value = obj_->array_table_.at(map_iter_->second).as_object();

        return KeyValue{key, value};
    }

    bool operator==(const TableIterator &other) const {
        // Do not check (version_ == other.version_)
        // because iter == end will never true if version is changed.
        return map_iter_ == other.map_iter_;
    }

    bool operator!=(const TableIterator &other) const {
        return !(*this == other);
    }

    bool is_end() const { return map_iter_ == map_iter_end_; }

    explicit operator bool() const { return !is_end(); }
};

class Object::TableRange {
private:
    TableIterator begin_;
    TableIterator end_;

public:
    TableRange(TableIterator begin, TableIterator end)
        : begin_(begin), end_(end) {}
    TableIterator begin() { return begin_; }
    TableIterator end() { return end_; }
};

inline Object::TableRange Object::iter_hash_table() {
    return TableRange(TableIterator(this, TableVisiblity::VISIBLE),
                      TableIterator(this, this->hash_table_.end(), TableVisiblity::VISIBLE));
}


class Object::ArrayIterator {
private:
    size_t version_;
    ObjectHolder obj_;
    std::vector<Object *>::iterator array_iter_;
    std::vector<Object *>::iterator array_iter_end_;

    /// Caller must hold lock of obj.
    explicit ArrayIterator(
        ObjectHolder obj, const std::vector<Object *>::iterator &map_iter,
        const std::vector<Object *>::iterator &array_iter_end)
        : obj_(obj) {
        version_ = obj->version_;
        array_iter_ = map_iter;
        array_iter_end_ = array_iter_end;
    }

    /// - check object version
    /// - check iterator not ended
    /// If checking is not ok, it will throw exception.
    /// Caller must hold lock of obj_.
    void check() const {
        if (version_ != obj_->version_) {
            throw BrokenIteratorError("Object changed while iteration");
        }

        if (is_end()) {
            throw std::out_of_range("iterator ended");
        }
    }

public:
    explicit ArrayIterator(ObjectHolder obj) : obj_(obj) {
        std::lock_guard lk{*obj_};
        version_ = obj->version_;

        array_iter_ = obj_->array_.begin();
        array_iter_end_ = obj_->array_.end();
    }

    ObjectHolder get() const {
        std::lock_guard lk{*obj_};
        check();

        return *array_iter_;
    }

    ArrayIterator next() const {
        std::lock_guard lk{*obj_};
        check();
        auto map_iter = array_iter_;
        return ArrayIterator(obj_, ++map_iter, array_iter_end_);
    }

    bool is_end() const { return array_iter_ == array_iter_end_; }

    explicit operator bool() const { return !is_end(); }
};

inline Object::ArrayIterator Object::iter_array() {
    return Object::ArrayIterator(this);
}

} // namespace ljf

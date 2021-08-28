#pragma once

#include "gtest/gtest.h"
#include "Object.hpp"
#include "runtime-internal.hpp"


namespace ljf
{

class BrokenIteratorError : public runtime_error
{
public:
    using runtime_error::runtime_error;
};

class Object::TableIterator
{
private:
    size_t version_;
    ObjectHolder obj_;
    std::unordered_map<std::string, size_t>::iterator map_iter_;
    std::unordered_map<std::string, size_t>::iterator map_iter_end_;

    /// Caller must hold lock of obj.
    explicit TableIterator(ObjectHolder obj,
                           const std::unordered_map<std::string, size_t>::iterator &map_iter,
                           const std::unordered_map<std::string, size_t>::iterator &map_iter_end) : obj_(obj)
    {
        version_ = obj->version_;
        map_iter_ = map_iter;
        map_iter_end_ = map_iter_end;
    }

    /// - check object version
    /// - check iterator not ended
    /// If checking is not ok, it will throw exception.
    /// Caller must hold lock of obj_.
    void check() const
    {
        if (version_ != obj_->version_)
        {
            throw BrokenIteratorError("Object changed while iteration");
        }

        if (is_end())
        {
            throw std::out_of_range("iterator ended");
        }
    }

public:
    struct KeyValue
    {
        std::string key;
        ObjectHolder value;
    };

    explicit TableIterator(ObjectHolder obj, TableVisiblity visiblity) : obj_(obj)
    {
        std::lock_guard lk{*obj_};
        version_ = obj->version_;
        auto &table = *[&]() {
            if (visiblity == TableVisiblity::visible)
            {
                return &obj_->hash_table_;
            }
            else
            {
                return &obj_->hidden_table_;
            }
        }();

        map_iter_ = table.begin();
        map_iter_end_ = table.end();
    }

    KeyValue get() const
    {
        std::lock_guard lk{*obj_};
        check();

        auto &key = map_iter_->first;
        Object *value = obj_->array_table_.at(map_iter_->second);
        return KeyValue{key, value};
    }

    TableIterator next() const
    {
        std::lock_guard lk{*obj_};
        check();
        auto map_iter = map_iter_;
        return TableIterator(obj_, ++map_iter, map_iter_end_);
    }

    std::string key() const
    {
        return get().key;
    }

    ObjectHolder value() const
    {
        return get().value;
    }
    bool is_end() const
    {
        return map_iter_ == map_iter_end_;
    }

    explicit operator bool() const
    {
        return !is_end();
    }
};
Object::TableIterator Object::iter_hash_table()
{
    return Object::TableIterator(this, TableVisiblity::visible);
}

Object::TableIterator Object::iter_hidden_table()
{
    return Object::TableIterator(this, TableVisiblity::hidden);
}

class Object::ArrayIterator
{
private:
    size_t version_;
    ObjectHolder obj_;
    std::vector<Object *>::iterator array_iter_;
    std::vector<Object *>::iterator array_iter_end_;

    /// Caller must hold lock of obj.
    explicit ArrayIterator(ObjectHolder obj,
                           const std::vector<Object *>::iterator &map_iter,
                           const std::vector<Object *>::iterator &array_iter_end) : obj_(obj)
    {
        version_ = obj->version_;
        array_iter_ = map_iter;
        array_iter_end_ = array_iter_end;
    }

    /// - check object version
    /// - check iterator not ended
    /// If checking is not ok, it will throw exception.
    /// Caller must hold lock of obj_.
    void check() const
    {
        if (version_ != obj_->version_)
        {
            throw BrokenIteratorError("Object changed while iteration");
        }

        if (is_end())
        {
            throw std::out_of_range("iterator ended");
        }
    }

public:
    explicit ArrayIterator(ObjectHolder obj) : obj_(obj)
    {
        std::lock_guard lk{*obj_};
        version_ = obj->version_;

        array_iter_ = obj_->array_.begin();
        array_iter_end_ = obj_->array_.end();
    }

    ObjectHolder get() const
    {
        std::lock_guard lk{*obj_};
        check();

        return *array_iter_;
    }

    ArrayIterator next() const
    {
        std::lock_guard lk{*obj_};
        check();
        auto map_iter = array_iter_;
        return ArrayIterator(obj_, ++map_iter, array_iter_end_);
    }

    bool is_end() const
    {
        return array_iter_ == array_iter_end_;
    }

    explicit operator bool() const
    {
        return !is_end();
    }
};
Object::ArrayIterator Object::iter_array()
{
    return Object::ArrayIterator(this);
}

// unittests for Iterator family (they are private data and methods)
TEST(ObjectIterator, HashTable)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_EQ(iter.get().key, "elem");
    EXPECT_EQ(iter.get().value, elem);
    auto iter_next = iter.next();
    ASSERT_TRUE(iter_next.is_end());
}

TEST(ObjectIterator, BrokenIter)
{
    ObjectHolder obj = ljf_new_object();
    ObjectHolder elem = ljf_new_object();
    set_object_to_table(obj.get(), "elem", elem.get());

    auto iter = obj->iter_hash_table();
    EXPECT_NO_THROW(iter.get());
    EXPECT_NO_THROW(iter.next());

    auto iter2 = obj->iter_hash_table();
    set_object_to_table(obj.get(), "elem2", elem.get());

    EXPECT_THROW(iter2.get(), BrokenIteratorError);
    EXPECT_THROW(iter2.next(), BrokenIteratorError);
}
} // namespace ljf


#include "gtest/gtest.h"
#include <gtest/gtest_prod.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Object.hpp"
#include "ObjectIterator.hpp"

template <> struct std::hash<ljf::TypeObject>;
namespace ljf {
class TypeObject {
private:
    bool circular_reference_ = false;
    std::unordered_map<std::string, std::shared_ptr<TypeObject>>
        hash_table_types_;
    std::unordered_map<std::string, std::shared_ptr<TypeObject>>
        hidden_table_types_;
    std::vector<std::shared_ptr<TypeObject>> array_types_;

private:
    friend std::shared_ptr<TypeObject> calculate_type(Object &obj);
    friend std::shared_ptr<TypeObject> calculate_type(Object &obj,
                                                      TypeCalcData &);
    FRIEND_TEST(calculate_type, TestCircularReference);

    explicit TypeObject(bool circular_reference)
        : circular_reference_(circular_reference) {}

    static std::shared_ptr<TypeObject> create_circular_reference_type_object() {
        return std::make_shared<TypeObject>(TypeObject(true));
    }

public:
    TypeObject() = default;

    bool is_circular_reference() const noexcept { return circular_reference_; }

    bool operator==(const TypeObject &other) const {
        return (circular_reference_ == other.circular_reference_) &&
               (hash_table_types_ == other.hash_table_types_) &&
               (hidden_table_types_ == other.hidden_table_types_) &&
               (array_types_ == other.array_types_);
    };

    size_t hash() const {
        if (circular_reference_) {
            // We return non 0 value because we distinguish circular
            // reference TypeObject from empty TypeObject
            return -1;
        }

        size_t hash = 0;

        for (auto &&[k, v] : hash_table_types_) {
            hash ^= std::hash<std::string>()(k);
            hash ^= v->hash();
        }

        for (auto &&[k, v] : hidden_table_types_) {
            hash ^= std::hash<std::string>()(k);
            hash ^= v->hash();
        }

        for (auto &&v : array_types_) {
            hash ^= v->hash();
        }

        return hash;
    }
};

struct TypeHash {
    size_t operator()(const std::shared_ptr<TypeObject> &type) const {
        return type->hash();
    }
};

struct TypeEqualTo {
    size_t operator()(const std::shared_ptr<TypeObject> &type1,
                      const std::shared_ptr<TypeObject> &type2) const {
        return *type1 == *type2;
    }
};

struct TypeSet {
    std::mutex mutex;
    std::unordered_set<std::shared_ptr<TypeObject>, TypeHash, TypeEqualTo> set;
};

static TypeSet global_type_set;
inline std::shared_ptr<TypeObject> calculate_type(Object &obj) {
    TypeCalcData data;
    return calculate_type(obj, data);
}

inline std::shared_ptr<TypeObject>
calculate_type(Object &obj, TypeCalcData &type_calc_data) {
    if (type_calc_data.objects_in_calculation.count(&obj) != 0) {
        return TypeObject::create_circular_reference_type_object();
    }

    type_calc_data.objects_in_calculation.insert(&obj);

    try {
        auto type_object = std::make_shared<TypeObject>();

        for (auto iter = obj.iter_hash_table(); !iter.is_end();
             iter.next()) {
            if (iter.key().is_object_key()) {
                throw "not implemented";
            }

            type_object->hash_table_types_[iter.key().get_key_as_c_str()] =
                iter.value()->calculate_type(type_calc_data);
        }

        for (auto iter = obj.iter_hidden_table(); !iter.is_end();
             iter.next()) {
            if (iter.key().is_object_key()) {
                throw "not implemented";
            }
            type_object->hidden_table_types_[iter.key().get_key_as_c_str()] =
                iter.value()->calculate_type(type_calc_data);
        }

        for (auto iter = obj.iter_array(); !iter.is_end(); iter = iter.next()) {
            type_object->array_types_.push_back(
                iter.get()->calculate_type(type_calc_data));
        }

        std::lock_guard lk{global_type_set.mutex};
        auto &type_set = global_type_set.set;
        auto [type_iter, is_inserted] = type_set.insert(type_object);
        return *type_iter;
    } catch (const BrokenIteratorError &e) {
        std::cerr << "calculate_type: BrokenIteratorError: " << e.what()
                  << '\n';
        return nullptr;
    }
}

} // namespace ljf

template <> struct std::hash<ljf::TypeObject> {
    size_t operator()(const ljf::TypeObject &type) const { return type.hash(); }
};

#include "ljf/ObjectWrapper.hpp"

#include "Object.hpp"

namespace ljf {
ObjectWrapper ObjectWrapper::get(const char *key) const {
    return holder_.get()->get(const_cast<char *>(key), LJFAttribute::C_STR_KEY);
}
} // namespace ljf

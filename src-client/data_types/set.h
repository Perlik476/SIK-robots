#ifndef ROBOTS_SET_H
#define ROBOTS_SET_H

#include "usings.h"
#include "bytes.h"
#include "uint.h"

template<class T>
requires isSerializable<T>
class Set: public Serializable {
    std::set<T> set;

public:
    Set() = default;

    explicit Set(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            set.insert(T(bytes));
        }
    }

    Bytes serialize() const override {
        auto length = static_cast<uint32_t>(set.size());

        Bytes list_content;
        for (auto &element: set) {
            list_content += element.serialize();
        }

        return Uint32(length).serialize() + list_content;
    }

    std::set<T> &get_set() {
        return set;
    }

    auto &get_set() const {
        return set;
    }
};

#endif //ROBOTS_SET_H

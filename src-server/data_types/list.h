#ifndef ROBOTS_LIST_H
#define ROBOTS_LIST_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"

template<class T>
requires isSerializable<T>
class List: public Serializable {
    std::vector<T> list;
public:
    List() = default;

    explicit List(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            list.push_back(T(bytes));
        }
    }

    Bytes serialize() const override {
        auto length = static_cast<uint32_t>(list.size());

        Bytes list_content;
        for (auto &element: list) {
            list_content += element.serialize();
        }

        return Uint32(length).serialize() + list_content;
    }

    auto &get_list() {
        return list;
    }

    auto &get_list() const {
        return list;
    }
};


template<class T>
requires isSerializable<T>
class PointerList: public Serializable {
    std::vector<std::shared_ptr<T>> list;
public:
    PointerList() = default;

    explicit PointerList(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            list.push_back(std::make_shared<T>(bytes));
        }
    }

    Bytes serialize() const override {
        auto length = static_cast<uint32_t>(list.size());

        Bytes list_content;
        for (auto &element: list) {
            list_content += element->serialize();
        }

        return Uint32(length).serialize() + list_content;
    }

    auto &get_list() {
        return list;
    }

    auto &get_list() const {
        return list;
    }
};


#endif //ROBOTS_LIST_H

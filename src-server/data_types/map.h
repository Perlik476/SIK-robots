#ifndef ROBOTS_MAP_H
#define ROBOTS_MAP_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"


template<isSerializable K, isSerializable T>
class Map: public Serializable {
    std::map<K, T> map;
public:
    Map() = default;

    explicit Map(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            isSerializable auto key = K(bytes);
            isSerializable auto value = T(bytes);
            map[key] = value;
        }
    }

    [[nodiscard]] Bytes serialize() const override {
        auto length = static_cast<uint32_t>(map.size());

        Bytes map_content;
        for (auto &[key, element] : map) {
            map_content += key.serialize();
            map_content += element.serialize();
        }

        return Uint32(length).serialize() + map_content;
    };

    auto &get_map() {
        return map;
    }

    auto &get_map() const {
        return map;
    }
};

#endif //ROBOTS_MAP_H

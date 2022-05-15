#ifndef ROBOTS_DEFINITIONS_H
#define ROBOTS_DEFINITIONS_H

#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include <utility>
#include <cstdint>
#include <cstring>
#include <memory>
#include <netinet/in.h>

using bytes_t = char *;

class Bytes: public std::vector<char> {
public:
    Bytes() = default;

    Bytes(bytes_t bytes, size_t length);

    Bytes(const std::string &string);

    Bytes(char byte);

    Bytes &operator+= (const Bytes &other);

    Bytes operator+ (const Bytes &other);
};

class Serializable {
public:
    virtual Bytes serialize() = 0;

    virtual ~Serializable() = default;
};

template<class T>
concept isSerializable = std::is_base_of_v<Serializable, T>;

template<class T>
concept isUint = std::is_integral_v<T>; // TODO

template<isUint T>
class Uint: public Serializable {
public:
    T value;

    explicit Uint(T value): value(value) {}

    Bytes serialize() override {
        if constexpr (std::is_same_v<T, uint8_t>) {
            return { (char) value };
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            char buffer[2];
            T value_to_net = htons(value);
            memcpy(buffer, &value_to_net, 2);
            return { buffer, 2 };
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            char buffer[4];
            T value_to_net = htonl(value);
            memcpy(buffer, &value_to_net, 4);
            return { buffer, 4 };
        }
    }
};

using player_id_t = Uint<uint8_t>;
using bomb_id_t = Uint<uint32_t>;
using score_t = Uint<uint32_t>;
using coordinate_t = Uint<uint16_t>;

enum Direction: char {
    Up,
    Right,
    Down,
    Left,
    Undefined
};

class String: public Serializable {
public:
    std::string string;

    String() = default;

    String(const std::string &string): string(string) {}

    Bytes serialize() override {
        auto length = static_cast<uint8_t>(string.length()); // TODO check length < 256

        return Uint<uint8_t>(length).serialize() + string;
    }
};

template<isSerializable T>
class List: public Serializable {
public:
    std::vector<std::shared_ptr<T>> list;

    Bytes serialize() override {
        auto length = static_cast<uint32_t>(list.size());

        Bytes list_content;
        for (auto &element : list) {
            list_content += element->serialize();
        }

        return Uint<uint32_t>(length).serialize() + list_content;
    }
};

template<isSerializable K, isSerializable T>
class Map: public Serializable {
public:
    std::map<std::shared_ptr<K>, std::shared_ptr<T>> map;
    Bytes serialize() override {
        auto length = static_cast<uint32_t>(map.size());

        Bytes map_content;
        for (auto &element : map) {
            map_content += element->serialize();
        }

        return Uint<uint32_t>(length).serialize() + map_content;
    };
};

class Player: public Serializable {
private:
    String name;
    String address;
public:
    Player(const String &name, const String &address): name(name), address(address) {}

    String get_name() { return name; }

    String get_address() { return address; }
};

#endif //ROBOTS_DEFINITIONS_H
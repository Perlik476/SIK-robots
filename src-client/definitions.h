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
    size_t index = 0;
public:
    Bytes() = default;

    Bytes(bytes_t bytes, size_t length);

    Bytes(std::string &string);

    Bytes(char byte);

    Bytes &operator+= (const Bytes &other);

    Bytes operator+ (const Bytes &other);

    char get_next_byte() {
        char value = (*this)[index];
        processed(1);
        return value;
    }

    char *get_pointer() {
        return data() + index;
    }

    void reset_pointer() {
        index = 0;
    }

    void processed(size_t number_of_bytes) {
        index += number_of_bytes;
    }
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

    Uint() = default;

    explicit Uint(T value): value(value) {}

    explicit Uint(Bytes &bytes) {
        if constexpr (std::is_same_v<T, uint8_t>) {
            memcpy(&value, bytes.get_pointer(), 1);
            bytes.processed(1);
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            memcpy(&value, bytes.get_pointer(), 2);
            value = ntohs(value);
            bytes.processed(2);
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            memcpy(&value, bytes.get_pointer(), 4);
            value = ntohl(value);
            bytes.processed(4);
        }
    }

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

using Uint8 = Uint<uint8_t>;
using Uint16 = Uint<uint16_t>;
using Uint32 = Uint<uint32_t>;

using PlayerId = Uint8;
using BombId = Uint32;
using Score = Uint32;
using Coordinate = Uint16;

class Position: public Serializable {
public:
    Uint16 x, y;

    Position() = default;

    Position(Uint16 &x, Uint16 &y) : x(x), y(y) {}

    explicit Position(Bytes &bytes) {
        x = Uint16(bytes);
        y = Uint16(bytes);
    }

    Bytes serialize() override {
        return x.serialize() + y.serialize();
    }
};

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

    String(const String &other) : string(other.string) {}

    explicit String(const std::string &string) : string(string) {}

    explicit String(Bytes &bytes) {
        Uint8 length = Uint8(bytes);
        for (size_t i = 0; i < length.value; i++) {
            string += bytes.get_next_byte();
        }
    }

    String &operator= (const String &other) {
        this->string = other.string;
        return *this;
    }

    Bytes serialize() override {
        auto length = static_cast<uint8_t>(string.length()); // TODO check length < 256

        return Uint8(length).serialize() + string;
    }
};

template<isSerializable T>
class List: public Serializable {
public:
    std::vector<std::shared_ptr<T>> list;

    List() = default;

    explicit List(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.value; i++) {
            list.push_back(std::make_shared<T>(T(bytes)));
        }
    }

    Bytes serialize() override {
        auto length = static_cast<uint32_t>(list.size());

        Bytes list_content;
        for (auto &element : list) {
            list_content += element->serialize();
        }

        return Uint32(length).serialize() + list_content;
    }
};

template<isSerializable K, isSerializable T>
class Map: public Serializable {
public:
    std::map<std::shared_ptr<K>, std::shared_ptr<T>> map;

    Map() = default;

    explicit Map(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.value; i++) {
            map[std::make_shared<K>(K(bytes))] = std::make_shared<T>(T(bytes));
        }
    }

    Bytes serialize() override {
        auto length = static_cast<uint32_t>(map.size());

        Bytes map_content;
        for (auto &element : map) {
            map_content += element->serialize();
        }

        return Uint32(length).serialize() + map_content;
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

    Bytes serialize() override {
        return name.serialize() + address.serialize();
    }
};

class Bomb: public Serializable {
public:
    Position position;
    Uint16 timer;

    Bomb(Position &position, Uint16 &timer): position(position), timer(timer) {}

    Bytes serialize() override {
        return position.serialize() + timer.serialize();
    }
};

class GameState {
public:
    String server_name;
    Uint8 players_count;
    Uint16 size_x;
    Uint16 size_t;
    Uint16 game_length;
    Uint16 explosion_radius;
    Uint16 bomb_timer;
    Map<PlayerId, Player> players;
    Uint16 turn;
    Map<PlayerId, Position> player_positions;
    List<Position> blocks;
    List<Bomb> bombs;
    List<Position> explosions;
    Map<PlayerId, Score> scores;
};

#endif //ROBOTS_DEFINITIONS_H
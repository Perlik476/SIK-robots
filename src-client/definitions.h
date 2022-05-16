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

    Bytes(std::string &string);

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

    Uint() = default;

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
private:
    String _server_name;
    Uint8 _players_count;
    Uint16 _size_x;
    Uint16 _size_t;
    Uint16 _game_length;
    Uint16 _explosion_radius;
    Uint16 _bomb_timer;
    Map<PlayerId, Player> _players;

    Uint16 _turn;
    Map<PlayerId, Position> _player_positions;
    List<Position> _blocks;
    List<Bomb> _bombs;
    List<Position> _explosions;
    Map<PlayerId, Score> _scores;
};

void deserialize(Bytes bytes, GameState &game_state);

#endif //ROBOTS_DEFINITIONS_H
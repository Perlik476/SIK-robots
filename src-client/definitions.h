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
#include <variant>

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

class GameState;

class Serializable {
public:
    virtual Bytes serialize() = 0;

    virtual ~Serializable() = default;
};

class Executable {
public:
    ~Executable() = default;

    virtual void execute(GameState &game_state) = 0;
};

template<class T>
concept isSerializable = std::is_base_of_v<Serializable, T>;

template<class T>
concept isExecutable = std::is_base_of_v<Executable, T>;

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

template<class T>
requires isSerializable<T> || isExecutable<T> // TODO
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
        if constexpr (isSerializable<T>) {
            auto length = static_cast<uint32_t>(list.size());

            Bytes list_content;
            for (auto &element: list) {
                list_content += element->serialize();
            }

            return Uint32(length).serialize() + list_content;
        }
        else {
            return {};
        }
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
        for (auto &[key, element] : map) {
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

    explicit Player(Bytes &bytes) {
        name = String(bytes);
        address = String(bytes);
    }

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

    explicit Bomb(Bytes &bytes) {
        position = Position(bytes);
        timer = Uint16(bytes);
    }

    Bytes serialize() override {
        return position.serialize() + timer.serialize();
    }
};

// TODO pozamieniać
using PlayersMap = Map<PlayerId, Player>;
using PlayersPositionsMap = Map<PlayerId, Position>;
using PlayerScoresMap = Map<PlayerId, Score>;
using BlocksList = List<Position>;
using BombsList = List<Bomb>;

class GameState {
public:
    String server_name;
    Uint8 players_count;
    Uint16 size_x;
    Uint16 size_y;
    Uint16 game_length;
    Uint16 explosion_radius;
    Uint16 bomb_timer;
    PlayersMap players;
    Uint16 turn;
    Map<PlayerId, Position> player_positions;
    List<Position> blocks;
    List<Bomb> bombs;
    List<Position> explosions;
    Map<PlayerId, Score> scores;
};

class BombPlacedEvent: public Executable {
    BombId id;
    Position position;

public:
    explicit BombPlacedEvent(Bytes &bytes) {
        id = BombId(bytes);
        position = Position(bytes);
    }

    void execute(GameState &game_state) override {
//        game_state.bombs.list
//      TODO
    }
};

class BombExplodedEvent: public Executable {
    BombId id;
    List<PlayerId> robots_destroyed;
    List<Position> blocks_destroyed;

public:
    explicit BombExplodedEvent(Bytes &bytes) {
        id = BombId(bytes);
        robots_destroyed = List<PlayerId>(bytes);
        blocks_destroyed = List<Position>(bytes);
    }

    void execute(GameState &game_state) override {
        // TODO
    }
};

class PlayerMovedEvent: public Executable {
    PlayerId id;
    Position position;

public:
    explicit PlayerMovedEvent(Bytes &bytes) {
        id = PlayerId(bytes);
        position = Position(bytes);
    }

    void execute(GameState &game_state) override {
        // TODO
    }
};

class BlockPlacedEvent: public Executable {
    Position position;

public:
    explicit BlockPlacedEvent(Bytes &bytes) {
        position = Position(bytes);
    }

    void execute(GameState &game_state) override {
        // TODO
    }
};

class Event: public Executable { // TODO
    enum Type: char {
        BombPlaced,
        BombExploded,
        PlayerMoved,
        BlockPlaced
    } type;

    using event_t = std::variant<BombPlacedEvent, BombExplodedEvent,
        PlayerMovedEvent, BlockPlacedEvent, bool>; // TODO

//    union {
//        BombPlacedEvent bomb_placed_event;
//        BombExplodedEvent bomb_exploded_event;
//        PlayerMovedEvent player_moved_event;
//        BlockPlacedEvent block_placed_event;
//    } event;

    event_t event = false;

public:
    ~Event() = default;
//        switch(type) {
//            case BombPlaced:
//                this->BombPlacedEvent::~bomb_placed_event();
//                break;
//            case BombExploded:
//                bomb_exploded_event = BombExplodedEvent(bytes);
//                break;
//            case PlayerMoved:
//                player_moved_event = PlayerMovedEvent(bytes);
//                break;
//            case BlockPlaced:
//                block_placed_event = BlockPlacedEvent(bytes);
//                break;
//            default:
//                // TODO
//                break;
//        }
//    }

    explicit Event(Bytes &bytes) {
        type = (Type) bytes.get_next_byte();
        switch(type) {
            case BombPlaced:
                event = BombPlacedEvent(bytes);
                break;
            case BombExploded:
                event = BombExplodedEvent(bytes);
                break;
            case PlayerMoved:
                event = PlayerMovedEvent(bytes);
                break;
            case BlockPlaced:
                event = BlockPlacedEvent(bytes);
                break;
            default:
                // TODO
                break;
        }
    }

//    Bytes serialize() override {
//        return {}; // TODO
//    }

    void execute(GameState &game_state) override {
        switch(type) {
            case BombPlaced:
                std::get<BombPlacedEvent>(event).execute(game_state);
                break;
            case BombExploded:
                std::get<BombExplodedEvent>(event).execute(game_state);
                break;
            case PlayerMoved:
                std::get<PlayerMovedEvent>(event).execute(game_state);
                break;
            case BlockPlaced:
                std::get<BlockPlacedEvent>(event).execute(game_state);
                break;
            default:
                // TODO
                break;
        }
    }
};

#endif //ROBOTS_DEFINITIONS_H
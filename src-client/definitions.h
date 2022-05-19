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
#include <iostream>
#include <mutex>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>

using bytes_t = char *;
using socket_tcp = std::shared_ptr<boost::asio::ip::tcp::socket>;
using socket_udp = std::shared_ptr<boost::asio::ip::udp::socket>;

class Bytes: public std::vector<char> {
protected:
    size_t index = 0;

    void processed(size_t number_of_bytes) {
        index += number_of_bytes;
    }
public:
    Bytes() = default;

    Bytes(bytes_t bytes, size_t length);

    Bytes(std::vector<char> &vec);

    Bytes(std::string &string);

    Bytes(const std::string &string);

    Bytes(char byte);

    Bytes &operator+= (const Bytes &other);

    Bytes operator+ (const Bytes &other);

    virtual char get_next_byte() {
        char value = (*this)[index];
        processed(1);
        return value;
    }

    std::vector<char> get_next_bytes(size_t n) {
        std::vector<char> bytes;
        for (size_t i = 0; i < n; i++) {
            bytes.push_back(get_next_byte());
        }
        return bytes;
    }

    bool is_end() {
        std::cout << index << "/" << size() << "\n";
        return index >= size();
    }

//    char *get_pointer() {
//        return data() + index;
//    }
//
    void reset_pointer() { // TODO
        index = 0;
    }
};

class BytesReceiver: public Bytes {
    socket_tcp socket;

    void listen() {
        boost::array<char, 128> buf;
        boost::system::error_code error;
        size_t size = socket->read_some(boost::asio::buffer(buf), error);
        for (size_t i = 0; i < size; i++) {
            push_back(buf[i]);
        }
    }
public:
    BytesReceiver(socket_tcp &socket) : socket(socket) {
        listen();
    }

    char get_next_byte() override {
        if (is_end()) {
            listen();
        }
        char value = (*this)[index];
        processed(1);
        return value;
    }
};

class GameState;

class Serializable {
public:
    virtual Bytes serialize() const = 0;

    virtual ~Serializable() = default;
};

class SocketsInfo {
public:
    boost::asio::ip::udp::socket socket_gui;
    socket_tcp socket_server;

};

class Executable {
public:
    virtual ~Executable() = default;

    virtual void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                         socket_tcp &socket_server, boost::asio::ip::udp::endpoint &gui_endpoint) = 0;
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

    constexpr Uint(T value): value(value) {}

    explicit Uint(Bytes &bytes) {
        if constexpr (std::is_same_v<T, uint8_t>) {
            memcpy(&value, &(bytes.get_next_bytes(1)[0]), 1);
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            memcpy(&value, &(bytes.get_next_bytes(2)[0]), 2);
            value = ntohs(value);
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            memcpy(&value, &(bytes.get_next_bytes(4)[0]), 4);
            value = ntohl(value);
        }
    }

    Bytes serialize() const override {
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

    bool operator<(Uint<T> const &other) const
    {
        return value < other.value;
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

    Position(uint16_t x, uint16_t y) : x(x), y(y) {}

    Position(Uint16 &x, Uint16 &y) : x(x), y(y) {}

//    Position(Position &position) : x(position.x), y(position.y) {}

    explicit Position(Bytes &bytes) {
        x = Uint16(bytes);
        y = Uint16(bytes);
    }

    Bytes serialize() const override {
        return x.serialize() + y.serialize();
    }

    Position up() {
        return Position(x.value, y.value + 1);
    }

    Position right() {
        return Position(x.value + 1, y.value);
    }

    Position down() {
        return Position(x.value, y.value - 1);
    }

    Position left() {
        return Position(x.value - 1, y.value);
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

    Bytes serialize() const override {
        auto length = static_cast<uint8_t>(string.length()); // TODO check length < 256

        return Uint8(length).serialize() + Bytes(string);
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

    Bytes serialize() const override {
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
    std::map<K, std::shared_ptr<T>> map;

    Map() = default;

    explicit Map(Bytes &bytes) {
        Uint32 length = Uint32(bytes);
        for (size_t i = 0; i < length.value; i++) {
            isSerializable auto key = K(bytes);
            auto value = std::make_shared<T>(T(bytes));
            map[key] = value;
        }
    }

    Bytes serialize() const override {
        auto length = static_cast<uint32_t>(map.size());

        Bytes map_content;
        for (auto &[key, element] : map) {
            map_content += key.serialize();
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

    Bytes serialize() const override {
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

    Bytes serialize() const override {
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
    std::map<BombId, std::shared_ptr<Bomb>> bombs_map;
    List<Position> explosions;
    Map<PlayerId, Score> scores;

    PlayerId my_id;

    std::shared_ptr<std::mutex> mutex;

    bool is_block_on_position(const Position &position) {
        for (const auto &block_position : blocks.list) {
            if (block_position->x.value == position.x.value
                && block_position->y.value == position.y.value) {
                return true;
            }
        }
        return false;
    }

    void place_bomb(Position position) {
        bombs.list.push_back(std::make_shared<Bomb>(
                position, bomb_timer));
    }

    void place_block(Position position) {
        if (!is_block_on_position(position)) {
            blocks.list.push_back(std::make_shared<Position>(position.x, position.y));
        }
    }

    void try_move(Direction direction, const std::shared_ptr<Position> &position) {
        switch(direction) {
            case Direction::Down:
                if (!is_block_on_position(position->down()) && position->y.value > 0) {
                    position->y.value--;
                }
                break;
            case Direction::Left:
                if (!is_block_on_position(position->left()) && position->x.value > 0) {
                    position->x.value--;
                }
                break;
            case Direction::Up:
                if (!is_block_on_position(position->up()) && position->y.value < size_y.value - 1) {
                    position->y.value++;
                }
                break;
            case Direction::Right:
                if (!is_block_on_position(position->right()) && position->x.value < size_x.value - 1) {
                    position->x.value++;
                }
                break;
            default:
                // TODO
                break;
        }
    }
};

class BombPlacedEvent: public Executable {
    BombId id;
    Position position;

public:
    explicit BombPlacedEvent(Bytes &bytes) {
        id = BombId(bytes);
        position = Position(bytes);
    }

    void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                 socket_tcp &socket_server,
                 boost::asio::ip::udp::endpoint &gui_endpoint) override {
        auto bomb = std::make_shared<Bomb>(position, game_state.bomb_timer);
        game_state.bombs.list.push_back(bomb);
        game_state.bombs_map[id] = bomb;
//      TODO
        std::cout << "BombPlaced\nid: " << id.value << ", x: " << position.x.value << ", y: " << position.y.value << "\n";
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

    void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                 socket_tcp &socket_server,
                 boost::asio::ip::udp::endpoint &gui_endpoint) override {
//        auto bomb = game_state.bombs_map[id];
//        remove(game_state.bombs.list.begin(), game_state.bombs.list.end(), bomb);
//        for (const auto &b : game_state.bombs.list) {
//            if (b == bomb) {
//            }
//        }
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

    void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                 socket_tcp &socket_server,
                 boost::asio::ip::udp::endpoint &gui_endpoint) override {
        // TODO
        game_state.player_positions.map[id] = std::make_shared<Position>(position.x.value, position.y.value);
        std::cout << "PlayerMoved: id: " << (int) id.value << ", x: " << position.x.value << ", y: " << position.y.value << "\n";
    }
};

class BlockPlacedEvent: public Executable {
    Position position;

public:
    explicit BlockPlacedEvent(Bytes &bytes) {
        position = Position(bytes);
    }

    void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                 socket_tcp &socket_server,
                 boost::asio::ip::udp::endpoint &gui_endpoint) override {
        game_state.blocks.list.push_back(std::make_shared<Position>(position.x.value, position.y.value));
        std::cout << "BlockPlaced: x: " << position.x.value << ", y: " << position.y.value << "\n";
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

    event_t event = false;

public:
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

    void execute(GameState &game_state, boost::asio::ip::udp::socket &socket_gui,
                 socket_tcp &socket_server,
                 boost::asio::ip::udp::endpoint &gui_endpoint) override {
        switch(type) {
            case BombPlaced:
                std::get<BombPlacedEvent>(event).execute(game_state, socket_gui, socket_server, gui_endpoint);
                break;
            case BombExploded:
                std::get<BombExplodedEvent>(event).execute(game_state, socket_gui, socket_server, gui_endpoint);
                break;
            case PlayerMoved:
                std::get<PlayerMovedEvent>(event).execute(game_state, socket_gui, socket_server, gui_endpoint);
                break;
            case BlockPlaced:
                std::get<BlockPlacedEvent>(event).execute(game_state, socket_gui, socket_server, gui_endpoint);
                break;
            default:
                // TODO
                break;
        }
    }
};

#endif //ROBOTS_DEFINITIONS_H
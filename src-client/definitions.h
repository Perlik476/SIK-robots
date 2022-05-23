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
#include <set>

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
        return index >= size();
    }
};

class BytesReceiver: public Bytes {
    socket_tcp socket;

    void listen() {
        boost::array<char, 128> buf{};
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

struct SocketsInfo {
    socket_udp gui_socket;
    boost::asio::ip::udp::endpoint gui_endpoint;
    socket_tcp server_socket;

    SocketsInfo(socket_udp &gui_socket, boost::asio::ip::udp::endpoint &gui_endpoint,
                socket_tcp &server_socket) : gui_socket(gui_socket), gui_endpoint(gui_endpoint),
                                            server_socket(server_socket) {}
};

class Executable {
public:
    virtual ~Executable() = default;

    virtual void execute([[maybe_unused]] GameState &game_state,
                         [[maybe_unused]] SocketsInfo &sockets_info) = 0;
};

template<class T>
concept isSerializable = std::is_base_of_v<Serializable, T>;

template<class T>
concept isExecutable = std::is_base_of_v<Executable, T>;

template<class T>
concept isUint = std::is_integral_v<T>; // TODO

template<isUint T>
class Uint: public Serializable {
    T value;
public:
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

    bool operator<(Uint<T> const &other) const {
        return value < other.get_value();
    }

    bool operator==(Uint<T> const &other) const {
        return value == other.get_value();
    }

    Uint<T> &operator+=(Uint<T> const &other) {
        value += other.get_value();
        return *this;
    }

    Uint<T> operator+(Uint<T> const &other) const {
        return *this += other;
    }

    auto get_value() {
        return value;
    }

    auto get_value() const {
        return value;
    }
};

using Uint8 = Uint<uint8_t>;
using Uint16 = Uint<uint16_t>;
using Uint32 = Uint<uint32_t>;

using PlayerId = Uint8;
using BombId = Uint32;
using Score = Uint32;
using Coordinate = Uint16;

enum Direction: char {
    Up,
    Right,
    Down,
    Left,
    Undefined
};

class Position: public Serializable {
    Uint16 x, y;
public:
    auto &get_x() { return x; }

    auto &get_y() { return y; }

    auto &get_x() const { return x; }

    auto &get_y() const { return y; }

    Position() = default;

    Position(uint16_t x, uint16_t y) : x(x), y(y) {}

    Position(Uint16 &x, Uint16 &y) : x(x), y(y) {}

//    Position(Position &position) : x(position.get_x()), y(position.get_y()) {}

    explicit Position(Bytes &bytes) {
        x = Uint16(bytes);
        y = Uint16(bytes);
    }

    Bytes serialize() const override {
        return x.serialize() + y.serialize();
    }

    Position up() const {
        return Position(x.get_value(), y.get_value() + 1);
    }

    Position right() const {
        return Position(x.get_value() + 1, y.get_value());
    }

    Position down() const {
        return Position(x.get_value(), y.get_value() - 1);
    }

    Position left() const {
        return Position(x.get_value() - 1, y.get_value());
    }

    Position next(Direction &direction) const {
        switch(direction) {
            case Direction::Up:
                return up();
            case Direction::Right:
                return right();
            case Direction::Down:
                return down();
            case Direction::Left:
                return left();
            default:
                return Position(0, 0); // TODO
        }
    }

    bool is_next_proper(Direction &direction, Uint16 size_x, Uint16 size_y) {
        switch(direction) {
            case Direction::Up:
                return y.get_value() < size_y.get_value() - 1;
            case Direction::Right:
                return x.get_value() < size_x.get_value() - 1;
            case Direction::Down:
                return y.get_value() > 0;
            case Direction::Left:
                return x.get_value() > 0;
            default:
                return false;
        }
    }

    bool operator<(Position const &other) const {
        return x < other.get_x() || (x == other.get_x() && y < other.get_y());
    }

    bool operator==(Position const &other) const {
        return x == other.get_x() && y == other.get_y();
    }
};

class String: public Serializable {
    std::string string;
public:
    auto &get_string() {
        return string;
    }

    auto &get_string() const {
        return string;
    }

    String() = default;

    explicit String(const std::string &string) : string(string) {}

    String(const String &string) : string(string.get_string()) {}

    explicit String(Bytes &bytes) {
        Uint8 length = Uint8(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            string += bytes.get_next_byte();
        }
    }

    String &operator= (const String &other) {
        this->string = other.get_string();
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
        if constexpr (isSerializable<T>) {
            auto length = static_cast<uint32_t>(list.size());

            Bytes list_content;
            for (auto &element: list) {
                list_content += element.serialize();
            }

            return Uint32(length).serialize() + list_content;
        }
        else {
            return {};
        }
    }

    auto &get_list() {
        return list;
    }

    auto &get_list() const {
        return list;
    }

    auto begin() {
        return list.begin();
    }

    auto end() {
        return list.end();
    }
};

template<class T>
requires isSerializable<T> || isExecutable<T> // TODO
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
        if constexpr (isSerializable<T>) {
            auto length = static_cast<uint32_t>(set.size());

            Bytes list_content;
            for (auto &element: set) {
                list_content += element.serialize();
            }

            return Uint32(length).serialize() + list_content;
        }
        else {
            return {};
        }
    }

    std::set<T> &get_set() {
        return set;
    }

    auto &get_set() const {
        return set;
    }
};

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

class Player: public Serializable {
private:
    String name;
    String address;
public:
    Player() = default;

    explicit Player(const String &name, const String &address): name(name), address(address) {}

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

    Bomb() = default;

    explicit Bomb(Position &position, Uint16 &timer): position(position), timer(timer) {}

    explicit Bomb(Bytes &bytes) {
        position = Position(bytes);
        timer = Uint16(bytes);
    }

    Bytes serialize() const override {
        return position.serialize() + timer.serialize();
    }

    bool operator==(const Bomb &other) const {
        return position == other.position && timer == other.timer;
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
    bool is_joined = false;
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
    std::map<BombId, Bomb> bombs_map;
    Set<Position> explosions;
    Map<PlayerId, Score> scores;
    std::map<PlayerId, bool> death_this_round;

    void prepare_for_turn() {
        explosions = Set<Position>();
        death_this_round.clear();
        for (auto [player_id, _] : scores.get_map()) {
            death_this_round[player_id] = false;
        }
    }

    void after_turn() {
        auto it = scores.get_map().begin();
        while (it != scores.get_map().end()) {
            if (death_this_round[it->first]) {
                std::cout << "PlayerId: " << (int) it->first.get_value() << " died.\n";
            }
            it->second += death_this_round[it->first];
            it++;
        }
    }

    void print() const {
        std::cout << "GameState:\n explosions:\n";
        for (auto &x : explosions.get_set()) {
            std::cout << "(" << x.get_x().get_value() << ", " << x.get_y().get_value() << ")\n";
        }
        std::cout << "scores:\n";
        for (auto &[x, y] : scores.get_map()) {
            std::cout << "scores[" << (int) x.get_value() << "] = " << y.get_value() << "\n";
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

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        auto bomb = Bomb(position, game_state.bomb_timer);
        game_state.bombs.get_list().push_back(bomb);
        game_state.bombs_map[id] = bomb;
//      TODO
        std::cout << "BombPlaced\nid: " << id.get_value() << ", x: " << position.get_x().get_value() << ", y: " << position.get_y().get_value() << "\n";
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

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        std::cout << "BombExploded!\n";
        auto bomb_exploded = game_state.bombs_map[id];
        auto bomb_position = bomb_exploded.position;

        auto it_bombs = game_state.bombs.get_list().begin();
        while (it_bombs != game_state.bombs.get_list().end()) {
            if (*it_bombs == bomb_exploded) {
                game_state.bombs.get_list().erase(it_bombs);
                break;
            }
            it_bombs++;
        }

        for (auto &player_id : robots_destroyed.get_list()) {
            game_state.death_this_round[player_id] = true;
        }

        for (auto &position : blocks_destroyed.get_list()) {
            auto it_blocks = game_state.blocks.get_list().begin();
            while (it_blocks != game_state.blocks.get_list().end()) {
                if (*it_blocks == position) {
                    game_state.blocks.get_list().erase(it_blocks);
                    break;
                }
                it_blocks++;
            }
        }

        game_state.explosions.get_set().insert(bomb_position);
        for (auto &block : blocks_destroyed.get_list()) {
            if (bomb_position == block) {
                return;
            }
        }

        for (size_t i = 0; i < 4; i++) {
            auto direction = static_cast<Direction>(i);
            auto current_position = bomb_position;
            bool cont = true;
            for (size_t r = 0; r < game_state.explosion_radius.get_value() && cont
                && current_position.is_next_proper(direction, game_state.size_x, game_state.size_y); r++) {

                current_position = current_position.next(direction);
                game_state.explosions.get_set().insert(current_position);

                for (auto &block : blocks_destroyed.get_list()) {
                    if (current_position == block) {
                        cont = false;
                        break;
                    }
                }
            }
        }
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

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        // TODO
        game_state.player_positions.get_map()[id] = Position(position.get_x().get_value(), position.get_y().get_value());
        std::cout << "PlayerMoved: id: " << (int) id.get_value() << ", x: " << position.get_x().get_value() << ", y: " << position.get_y().get_value() << "\n";
    }
};

class BlockPlacedEvent: public Executable {
    Position position;

public:
    explicit BlockPlacedEvent(Bytes &bytes) {
        position = Position(bytes);
    }

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        game_state.blocks.get_list().push_back(position);
        std::cout << "BlockPlaced: x: " << position.get_x().get_value() << ", y: " << position.get_y().get_value() << "\n";
        // TODO
    }
};

class Event: public Executable { // TODO
    enum Type: char {
        BombPlaced,
        BombExploded,
        PlayerMoved,
        BlockPlaced,
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

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        switch(type) {
            case BombPlaced:
                std::get<BombPlacedEvent>(event).execute(game_state, sockets_info);
                break;
            case BombExploded:
                std::get<BombExplodedEvent>(event).execute(game_state, sockets_info);
                break;
            case PlayerMoved:
                std::get<PlayerMovedEvent>(event).execute(game_state, sockets_info);
                break;
            case BlockPlaced:
                std::get<BlockPlacedEvent>(event).execute(game_state, sockets_info);
                break;
            default:
                // TODO
                break;
        }
    }
};

#endif //ROBOTS_DEFINITIONS_H
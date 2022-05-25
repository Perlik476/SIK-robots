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
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>
#include <set>
#include <mutex>
#include <condition_variable>

using bytes_t = char *;
using udp = boost::asio::ip::udp;
using tcp = boost::asio::ip::tcp;
using server_socket_t = std::shared_ptr<tcp::socket>;
using gui_socket_t = std::shared_ptr<udp::socket>;
using gui_endpoint_t = udp::endpoint;

class ThreadsInfo {
    std::mutex mutex;
    std::condition_variable condition_variable;
    bool should_exit = false;
public:
    auto &get_mutex() {
        return mutex;
    }

    auto &get_condition_variable() {
        return condition_variable;
    }

    bool get_should_exit() const {
        return should_exit;
    }

    void set_should_exit() {
        should_exit = true;
        condition_variable.notify_all();
    }
};


class Arguments {
    static bool is_proper_string(std::string &s) {
        return s.length() < 256;
    }

public:
    std::string player_name;
    uint16_t port;
    std::string gui_address;
    std::string server_address;

    std::string gui_address_pure;
    std::string gui_port;
    std::string server_address_pure;
    std::string server_port;

    static std::pair<std::string, std::string> process_address(std::string &address) {
        int length = (int) address.length();
        std::cout << length << "\n";

        std::string address_pure, port_str;

        for (int i = length - 1; i >= 0; i--) {
            char c = address[(size_t) i];
            if (c == ':') {
                for (size_t j = 0; j < (size_t) i; j++) {
                    address_pure += address[j];
                }
                break;
            }
            if (c < '0' || c > '9') {
                exit(1);
            }
            port_str += c;
        }
        std::reverse(port_str.begin(), port_str.end());

        return { address_pure, port_str };
    }

    bool check_correctness() {
        return is_proper_string(player_name);
    }

public:
    Arguments(std::string player_name, uint16_t port, std::string gui_address, std::string server_address)
            : player_name(std::move(player_name)), port(port), gui_address(std::move(gui_address)),
              server_address(std::move(server_address)) {

        auto pair = process_address(this->gui_address);
        gui_address_pure = pair.first;
        gui_port = pair.second;

        pair = process_address(this->server_address);
        server_address_pure = pair.first;
        server_port = pair.second;
    }
};

class SocketsInfo {
    boost::asio::io_context io_context;
    gui_socket_t gui_socket;
    gui_endpoint_t gui_endpoint;
    server_socket_t server_socket;

public:
    SocketsInfo() = default;

    SocketsInfo(Arguments &arguments) {
        udp::resolver resolver(io_context);
        try {
            gui_endpoint = *resolver.resolve(arguments.gui_address_pure, arguments.gui_port).begin();
        }
        catch (std::exception &exception) {
            std::cerr << "Resolving GUI address failed." << std::endl;
            throw exception;
        }

        try {
            gui_socket = std::make_shared<udp::socket>(io_context, udp::endpoint(udp::v6(), arguments.port));
        }
        catch (std::exception &exception) {
            std::cerr << "Socket for GUI communications could not be opened." << std::endl;
            throw exception;
        }

        tcp::resolver resolver_tcp(io_context);
        tcp::endpoint endpoints;
        try {
            endpoints = *resolver_tcp.resolve(arguments.server_address_pure, arguments.server_port);
        }
        catch (std::exception &exception) {
            std::cerr << "Resolving server address failed." << std::endl;
            throw exception;
        }

        try {
            server_socket = std::make_shared<tcp::socket>(io_context);
            server_socket->connect(endpoints);
            tcp::no_delay option(true);
            server_socket->set_option(option);
        }
        catch (std::exception &exception) {
            std::cerr << "Connecting with server failed." << std::endl;
            throw exception;
        }
    }

    auto &get_gui_socket() {
        return gui_socket;
    }

    auto &get_server_socket() {
        return server_socket;
    }

    auto &get_gui_endpoint() {
        return gui_endpoint;
    }
};

class BytesDeserializationException : public std::exception {
public:
    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
        return "Message deserialization failed.";
    }
};

class Bytes {
protected:
    std::vector<char> vector;

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
        if (is_end()) {
            throw BytesDeserializationException();
        }
        char value = vector[index];
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
        return index >= vector.size();
    }

    const auto &get_vector() const {
        return vector;
    }
};

class BytesReceiver: public Bytes {
    server_socket_t socket = nullptr;

    void listen() {
        boost::array<char, 128> buffer{};
        std::cout << "read_some" << std::endl;
        size_t size = socket->read_some(boost::asio::buffer(buffer));
        for (size_t i = 0; i < size; i++) {
            vector.push_back(buffer[i]);
        }
    }
public:
    BytesReceiver() = default;

    BytesReceiver(server_socket_t &socket) : socket(socket) {
        listen();
    }

    char get_next_byte() override {
        if (is_end()) {
            listen();
        }
        char value = vector[index];
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

    Uint<T> &operator-=(Uint<T> const &other) {
        value -= other.get_value();
        return *this;
    }

    Uint<T> operator-(Uint<T> const &other) const {
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
using players_count_t = Uint8;
using game_length_t = Uint16;
using explosion_radius_t = Uint16;
using bomb_timer_t = Uint16;
using turn_t = Uint16;

enum Direction: char {
    Up,
    Right,
    Down,
    Left,
    Undefined
};

class Position: public Serializable {
    Coordinate x, y;
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
        x = Coordinate(bytes);
        y = Coordinate(bytes);
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
                throw std::invalid_argument("Argument is not a proper direction.");
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
        auto length = static_cast<uint8_t>(string.length());

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
            throw std::logic_error("Cannot serialize non-serializable type.");
        }
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
class PointersList: public Serializable {
    std::vector<std::shared_ptr<T>> list;
public:
    PointersList() = default;

    explicit PointersList(Bytes &bytes) {
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
using players_t = Map<PlayerId, Player>;
using players_positions_t = Map<PlayerId, Position>;
using players_scores_t = Map<PlayerId, Score>;
using blocks_t = List<Position>;
using bombs_t = List<Bomb>;

class GameState {
public:
    bool is_joined = false;
    String server_name;
    Uint8 players_count;
    Coordinate size_x;
    Coordinate size_y;
    game_length_t game_length;
    explosion_radius_t explosion_radius;
    bomb_timer_t bomb_timer;
    players_t players;
    turn_t turn;
    Map<PlayerId, Position> player_positions;
    List<Position> blocks;
    PointersList<Bomb> bombs;
    std::map<BombId, std::shared_ptr<Bomb>> bombs_map;
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
        std::cout << "GameState:" << std::endl;
        std::cout << "bombs:\n";
        for (auto &bomb : bombs.get_list()) {
            std::cout << "(" << bomb->position.get_x().get_value() << ", " << bomb->position.get_y().get_value() << "), "
                << bomb->timer.get_value() << "\n";
        }
        std::cout << "bombs map:\n";
        for (auto &[id, bomb] : bombs_map) {
            std::cout << id.get_value() << ": (" << bomb->position.get_x().get_value() << ", " << bomb->position.get_y().get_value() << "), "
                      << bomb->timer.get_value() << "\n";
        }
        std::cout << "blocks:\n";
        for (auto &block_position : blocks.get_list()) {
            std::cout << "(" << block_position.get_x().get_value() << ", " << block_position.get_y().get_value() << ")\n";
        }
        std::cout << "explosions:\n";
        for (auto &x : explosions.get_set()) {
            std::cout << "(" << x.get_x().get_value() << ", " << x.get_y().get_value() << ")\n";
        }
        std::cout << "scores:\n";
        for (auto &[x, y] : scores.get_map()) {
            std::cout << "scores[" << (int) x.get_value() << "] = " << y.get_value() << "\n";
        }
    }

    void reset() {
        turn = 0;
        players.get_map().clear();
        player_positions.get_map().clear();
        blocks.get_list().clear();
        bombs.get_list().clear();
        bombs_map.clear();
        explosions.get_set().clear();
        scores.get_map().clear();
        death_this_round.clear();
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
        auto bomb = std::make_shared<Bomb>(position, game_state.bomb_timer);
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

    void remove_bomb(GameState &game_state, std::shared_ptr<Bomb> &bomb) {
        auto &bombs = game_state.bombs.get_list();
        auto it_bombs = bombs.begin();

        while (it_bombs != bombs.end()) {
            if (*it_bombs == bomb) {
                bombs.erase(it_bombs);
                break;
            }
            it_bombs++;
        }
    }

    void set_dead_players(GameState &game_state) {
        for (auto &player_id : robots_destroyed.get_list()) {
            game_state.death_this_round[player_id] = true;
        }
    }

    void remove_blocks(GameState &game_state) {
        auto &blocks_positions = game_state.blocks.get_list();

        for (auto &position : blocks_destroyed.get_list()) {
            auto it_blocks_positions = blocks_positions.begin();

            while (it_blocks_positions != blocks_positions.end()) {
                if (*it_blocks_positions == position) {
                    blocks_positions.erase(it_blocks_positions);
                    break;
                }

                it_blocks_positions++;
            }
        }
    }

    bool is_explosion_inside_a_block(GameState &game_state, Position &bomb_position) {
        game_state.explosions.get_set().insert(bomb_position);

        for (auto &block : blocks_destroyed.get_list()) {
            if (bomb_position == block) {
                return true;
            }
        }

        return false;
    }

    void add_explosions(GameState &game_state, Position &bomb_position) {
        for (size_t i = 0; i < 4; i++) {
            auto direction = static_cast<Direction>(i);
            auto current_position = bomb_position;
            bool do_continue = true;

            for (size_t r = 0; r < game_state.explosion_radius.get_value() && do_continue
                               && current_position.is_next_proper(direction, game_state.size_x, game_state.size_y); r++) {

                current_position = current_position.next(direction);
                game_state.explosions.get_set().insert(current_position);

                for (auto &block : blocks_destroyed.get_list()) {
                    if (current_position == block) {
                        do_continue = false;
                        break;
                    }
                }
            }
        }
    }
public:
    explicit BombExplodedEvent(Bytes &bytes) {
        id = BombId(bytes);
        robots_destroyed = List<PlayerId>(bytes);
        blocks_destroyed = List<Position>(bytes);
    }

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        std::cout << "BombExploded!\n";
        auto bomb_exploded = game_state.bombs_map[id];
        auto bomb_position = bomb_exploded->position;

        remove_bomb(game_state, bomb_exploded);

        set_dead_players(game_state);

        remove_blocks(game_state);

        if (!is_explosion_inside_a_block(game_state, bomb_position)) {
            add_explosions(game_state, bomb_position);
        }
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

class Event: public Executable {
    enum Type: char {
        BombPlaced,
        BombExploded,
        PlayerMoved,
        BlockPlaced,
    } type;

    using event_t = std::variant<BombPlacedEvent, BombExplodedEvent,
        PlayerMovedEvent, BlockPlacedEvent>;

    event_t event;

    event_t get_event(Bytes &bytes) {
        type = (Type) bytes.get_next_byte();
        switch(type) {
            case BombPlaced:
                return BombPlacedEvent(bytes);
                break;
            case BombExploded:
                return BombExplodedEvent(bytes);
                break;
            case PlayerMoved:
                return PlayerMovedEvent(bytes);
                break;
            case BlockPlaced:
                return BlockPlacedEvent(bytes);
                break;
            default:
                throw BytesDeserializationException();
        }
    }
public:
    explicit Event(Bytes &bytes) : event(get_event(bytes)) {}

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
        }
    }
};

#endif //ROBOTS_DEFINITIONS_H
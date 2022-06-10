#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"
#include "string.h"
#include "list.h"
#include "set.h"
#include "map.h"
#include "player.h"
#include "position.h"
#include "bomb.h"
#include "arguments.h"
#include "usings.h"
#include "events.h"
#include "action.h"
#include <random>

class ServerMessage;
class ClientMessage;

class ClientState {
    uint64_t game_number = 0;
    int accepted_players_sent = 0;
    bool game_started_sent = false;
    int turns_sent = 0;
    bool game_ended_sent = false;

public:
    ClientState() = default;

    int get_accepted_players_sent() { return accepted_players_sent; }
    int get_turns_sent() { return turns_sent; }
    bool get_game_started_sent() { return game_started_sent; }
    bool get_game_ended_sent() { return game_ended_sent; }
    uint64_t get_game_number() { return game_number; }

    void increase_accepted_players_sent(int inc) { accepted_players_sent += inc; }
    void increase_turns_sent(int inc) { turns_sent += inc; }
    void set_game_started_sent() { game_started_sent = true; }
    void set_game_ended_sent() { game_ended_sent = true; }
    void set_game_number(uint64_t new_game_number) { this->game_number = new_game_number; }

    void reset() {
        accepted_players_sent = 0;
        turns_sent = 0;
        game_started_sent = false;
        game_ended_sent = false;
    }
};

class GameState {
    bool is_started = false;
    bool is_ended = false;
    uint8_t next_player_id = 0;
    uint64_t game_number = 0;

    std::atomic_bool is_sending = false;
    std::mutex mutex;
    std::condition_variable main_loop;
    std::condition_variable sending_condition;
    std::condition_variable sending_ended;
    std::atomic_int how_many_to_send = 0;

    uint16_t initial_blocks;
    uint32_t seed;
    uint64_t turn_duration;

    std::minstd_rand random;

    String server_name;
    players_count_t players_count;
    coordinate_t size_x;
    coordinate_t size_y;
    game_length_t game_length;
    explosion_radius_t explosion_radius;
    bomb_timer_t bomb_timer;
    players_t players;
    turn_t turn = 0;
    players_positions_t player_positions;
    blocks_t blocks;
    bombs_t bombs;
    std::map<bomb_id_t, std::shared_ptr<Bomb>> bombs_map;
    players_scores_t scores;
    std::set<player_id_t> player_deaths_this_round;
    std::set<Position> blocks_destroyed_this_round;

    bomb_id_t next_bomb_id = 0;

    std::map<player_id_t, std::shared_ptr<Action>> players_action;
    std::vector<std::shared_ptr<ServerMessage>> accepted_players_to_send;
    std::vector<std::shared_ptr<ServerMessage>> turn_messages;

    Position get_random_position() {
        return Position(static_cast<uint16_t>(random() % size_x.get_value()),
                        static_cast<uint16_t>(random() % size_y.get_value()));
    }

    void start_game();

    void add_explosion(const Position &bomb_position, List<player_id_t> &robots_destroyed,
                       List<Position> &blocks_destroyed);

    void reset() {
        // TODO
        turn = 0;
        next_player_id = 0;
        players.get_map().clear();
        player_positions.get_map().clear();
        blocks.get_set().clear();
        bombs.get_list().clear();
        bombs_map.clear();
        scores.get_map().clear();
        player_deaths_this_round.clear();
        blocks_destroyed_this_round.clear();
        next_bomb_id = 0;
        players_action.clear();

        accepted_players_to_send.clear();
        turn_messages.clear();

        is_started = false;
        is_ended = false;
    }

public:
    friend MoveAction;
    friend PlaceBombAction;
    friend PlaceBlockAction;

    GameState(std::shared_ptr<Arguments> &arguments) : initial_blocks(arguments->initial_blocks),
        seed(arguments->seed), turn_duration(arguments->turn_duration), server_name(arguments->server_name),
        players_count(arguments->players_count), size_x(arguments->size_x),
        size_y(arguments->size_y), game_length(arguments->game_length), explosion_radius(arguments->explosion_radius),
        bomb_timer(arguments->bomb_timer) {

        random = std::minstd_rand(seed);
    }

    void try_add_player(const String &player_name, const String &address);

    void set_action(std::shared_ptr<Action> &action, socket_t &socket) {
        std::unique_lock<std::mutex> lock(mutex);
        auto endpoint = socket_to_string(socket);
        for (auto &[id, player]: players.get_map()) {
            if (player.get_address().get_string() == endpoint.get_string()) {
                players_action[id] = action;
                return;
            }
        }
    }

    void next_turn();

    void send_next();

    void wait();

    std::vector<std::shared_ptr<ServerMessage>> get_messages_to_send(ClientState &client_state);

    auto &get_server_name() const { return server_name; }
    auto &get_players_count() const { return players_count; }
    auto &get_size_x() const { return size_x; }
    auto &get_size_y() const { return size_y; }
    auto &get_game_length() const { return game_length; }
    auto &get_explosion_radius() const { return explosion_radius; }
    auto &get_bomb_timer() const { return bomb_timer; }
    auto &get_players() const { return players; }
    auto &get_turn() const { return turn; }
    auto &get_player_positions() const { return player_positions; }
    auto &get_blocks() const { return blocks; }
    auto &get_bombs() const { return bombs; }
    auto &get_scores() const { return scores; }
    auto &get_turn_duration() const { return turn_duration; }

    bool get_is_started() const { return is_started; }
};

#endif //ROBOTS_GAME_STATE_H

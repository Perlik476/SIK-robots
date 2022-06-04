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

class ServerMessage;
class ClientMessage;

class GameState {
    bool is_started = false;
    uint8_t next_player_id = 0;

    std::atomic_bool is_sending = false;
    std::mutex mutex;
    std::condition_variable sending_condition;
    std::condition_variable sending_ended;
    std::atomic_int how_many_to_send = 0;

    std::set<uint8_t> accepted_players_to_send;

    uint16_t initial_blocks;
    uint32_t seed;
    uint64_t turn_duration;

    String server_name;
    players_count_t players_count;
    coordinate_t size_x;
    coordinate_t size_y;
    game_length_t game_length;
    explosion_radius_t explosion_radius;
    bomb_timer_t bomb_timer;
    players_t players;
    turn_t turn;
    players_positions_t player_positions;
    blocks_t blocks;
    bombs_t bombs;
    std::map<BombId, std::shared_ptr<Bomb>> bombs_map;
    explosions_t explosions;
    players_scores_t scores;
    std::map<player_id_t, bool> death_this_round;

    std::map<player_id_t, std::shared_ptr<ClientMessage>> players_action;

public:
    GameState(std::shared_ptr<Arguments> &arguments) : server_name(arguments->server_name),
        players_count(arguments->players_count), size_x(arguments->size_x), size_y(arguments->size_y),
        game_length(arguments->game_length), explosion_radius(arguments->explosion_radius),
        bomb_timer(arguments->bomb_timer), initial_blocks(arguments->initial_blocks), seed(arguments->seed),
        turn_duration(arguments->turn_duration) {}

    void set_action(player_id_t &player_id, std::shared_ptr<ClientMessage> &client_message) {
        players_action[player_id] = client_message;
    }

    void try_add_player(const String &player_name, const String &address) {
        if (players.get_map().size() == players_count.get_value()) {
            return;
        }

        players.get_map()[next_player_id] = Player(player_name, address);
        accepted_players_to_send.insert(next_player_id);
        next_player_id++;
    }

    void before_turn() {
        explosions = Set<Position>();
        death_this_round.clear();
        for (auto [player_id, _] : scores.get_map()) {
            death_this_round[player_id] = false;
        }
        for (auto &bomb : bombs.get_list()) {
            bomb->timer -= 1;
        }
    }

    void after_turn() {
        auto it = scores.get_map().begin();
        while (it != scores.get_map().end()) {
            it->second += death_this_round[it->first];
            it++;
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

    void send_next();

    std::vector<std::shared_ptr<ServerMessage>> get_messages();

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
    auto &get_explosions() const { return explosions; }
    auto &get_scores() const { return scores; }
    auto &get_turn_duration() const { return turn_duration; }
};

#endif //ROBOTS_GAME_STATE_H

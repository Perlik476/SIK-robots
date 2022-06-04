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

using score_t = Uint32;
using players_count_t = Uint8;
using game_length_t = Uint16;
using explosion_radius_t = Uint16;
using bomb_timer_t = Uint16;
using turn_t = Uint16;

using players_t = Map<player_id_t, Player>;
using players_positions_t = Map<player_id_t, Position>;
using players_scores_t = Map<player_id_t, score_t>;
using blocks_t = List<Position>;
using bombs_t = PointerList<Bomb>;
using explosions_t = Set<Position>;

class GameState {
    bool is_joined = false;
    std::string player_name;

    String server_name;
    players_count_t players_count;
    Coordinate size_x;
    Coordinate size_y;
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

public:
    // Messages from server.
    friend class TurnMessage;
    friend class GameStartedMessage;
    friend class AcceptedPlayerMessage;
    friend class HelloMessage;
    friend class GameEndedMessage;

    // Events from TurnMessage.
    friend class BombPlacedEvent;
    friend class BombExplodedEvent;
    friend class PlayerMovedEvent;
    friend class BlockPlacedEvent;

    GameState(std::string &player_name) : player_name(player_name) {}

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

    void set_is_joined(bool value) { is_joined = value; }

    bool get_is_joined() const { return is_joined; }
    auto &get_player_name() const { return player_name; }
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
};

#endif //ROBOTS_GAME_STATE_H
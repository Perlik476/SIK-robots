#ifndef ROBOTS_GAME_STATE_H
#define ROBOTS_GAME_STATE_H

#include "usings.h"
#include "bytes.h"
#include "uint.h"
#include "string.h"
#include "list.h"
#include "set.h"
#include "map.h"
#include "player.h"
#include "position.h"
#include "bomb.h"
// TODO

using Score = Uint32;
using players_count_t = Uint8;
using game_length_t = Uint16;
using explosion_radius_t = Uint16;
using bomb_timer_t = Uint16;
using turn_t = Uint16;

using players_t = Map<PlayerId, Player>;
using players_positions_t = Map<PlayerId, Position>;
using players_scores_t = Map<PlayerId, Score>;
using blocks_t = List<Position>;
using bombs_t = PointersList<Bomb>;

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
//                std::cout << "PlayerId: " << (int) it->first.get_value() << " died.\n";
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

#endif //ROBOTS_GAME_STATE_H

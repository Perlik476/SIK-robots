#ifndef ROBOTS_USINGS_H
#define ROBOTS_USINGS_H

#include "uint.h"
#include "player.h"
#include "position.h"
#include "map.h"
#include "bomb.h"
#include "list.h"
#include "set.h"

using score_t = Uint32;
using players_count_t = Uint8;
using game_length_t = Uint16;
using explosion_radius_t = Uint16;
using bomb_timer_t = Uint16;
using turn_t = Uint16;

using players_t = Map<player_id_t, Player>;
using players_positions_t = Map<player_id_t, Position>;
using players_scores_t = Map<player_id_t, score_t>;
using blocks_t = Set<Position>;
using bombs_t = PointerList<Bomb>;

#endif //ROBOTS_USINGS_H

#include "action.h"
#include "../includes.h"

std::shared_ptr<Event> MoveAction::execute(GameState *game_state, uint8_t &player_id) {
    auto &position = game_state->player_positions.get_map()[player_id];
    if (position.is_next_proper(direction, game_state->get_size_x(), game_state->get_size_y())) {
        position = position.next(direction);

        auto id_temp = player_id_t(player_id);

        return std::make_shared<PlayerMovedEvent>(id_temp, position);
    }
    else {
        return nullptr;
    }
}

std::shared_ptr<Event> PlaceBombAction::execute(GameState *game_state, uint8_t &player_id) {
    auto &position = game_state->player_positions.get_map()[player_id];
    auto &bombs = game_state->bombs.get_list();
    auto &bombs_map = game_state->bombs_map;

    bomb_id_t bomb_id = 0;
    for (auto &[id, ptr] : bombs_map) {
        if (id.get_value() > bomb_id.get_value()) {
            bomb_id = id;
        }
    }

    auto bomb = std::make_shared<Bomb>(position, game_state->bomb_timer);
    bombs.push_back(bomb);
    bombs_map[bomb_id] = bomb;

    return std::make_shared<BombPlacedEvent>(bomb_id, position);
}

std::shared_ptr<Event> PlaceBlockAction::execute(GameState *game_state, uint8_t &player_id) {
    auto &position = game_state->player_positions.get_map()[player_id];
    auto &blocks = game_state->blocks.get_set();
    if (!blocks.contains(position)) {
        blocks.insert(position);
        return std::make_shared<BlockPlacedEvent>(position);
    }
    else {
        return nullptr;
    }
}

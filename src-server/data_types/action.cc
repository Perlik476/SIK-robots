#include "action.h"
#include "../includes.h"

std::shared_ptr<Event> MoveAction::execute(GameState *game_state, uint8_t &player_id) {
    auto &position = game_state->player_positions.get_map()[player_id];
    position = position.next(direction);
    auto id_temp = player_id_t(player_id);

    return std::make_shared<PlayerMovedEvent>(id_temp, position);
}

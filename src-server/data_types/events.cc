//#include "events.h"
//#include "game_state.h"
//
//void BombPlacedEvent::execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) {
//    auto bomb = std::make_shared<Bomb>(position, game_state.bomb_timer);
//    game_state.bombs.get_list().push_back(bomb);
//    game_state.bombs_map[id] = bomb;
//}
//
//void BombExplodedEvent::remove_bomb(GameState &game_state, std::shared_ptr<Bomb> &bomb) {
//    auto &bombs = game_state.bombs.get_list();
//    auto it_bombs = bombs.begin();
//
//    while (it_bombs != bombs.end()) {
//        if (*it_bombs == bomb) {
//            bombs.erase(it_bombs);
//            break;
//        }
//        it_bombs++;
//    }
//}
//
//void BombExplodedEvent::set_dead_players(GameState &game_state) {
//    for (auto &player_id : robots_destroyed.get_list()) {
//        game_state.player_deaths_this_round[player_id] = true;
//    }
//}
//
//void BombExplodedEvent::remove_blocks(GameState &game_state) {
//    auto &blocks_positions = game_state.blocks.get_list();
//
//    for (auto &position : blocks_destroyed.get_list()) {
//        auto it_blocks_positions = blocks_positions.begin();
//
//        while (it_blocks_positions != blocks_positions.end()) {
//            if (*it_blocks_positions == position) {
//                blocks_positions.erase(it_blocks_positions);
//                break;
//            }
//
//            it_blocks_positions++;
//        }
//    }
//}
//
//bool BombExplodedEvent::is_explosion_inside_a_block(GameState &game_state, Position &bomb_position) {
//    game_state.explosions.get_set().insert(bomb_position);
//
//    return std::ranges::any_of(blocks_destroyed.get_list(),
//                        [&bomb_position](auto &block_position){ return bomb_position == block_position; });
//}
//
//void BombExplodedEvent::add_explosions(GameState &game_state, Position &bomb_position) {
//    for (size_t i = 0; i < 4; i++) {
//        auto direction = static_cast<Direction>(i);
//        auto current_position = bomb_position;
//        bool do_continue = true;
//
//        for (size_t r = 0; r < game_state.explosion_radius.get_value() && do_continue
//                   && current_position.is_next_proper(direction, game_state.size_x, game_state.size_y); r++) {
//
//            current_position = current_position.next(direction);
//            game_state.explosions.get_set().insert(current_position);
//
//            for (auto &block_position : blocks_destroyed.get_list()) {
//                if (current_position == block_position) {
//                    do_continue = false;
//                    break;
//                }
//            }
//        }
//    }
//}
//
//void BombExplodedEvent::execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) {
//    auto bomb_exploded = game_state.bombs_map[id];
//    auto bomb_position = bomb_exploded->position;
//
//    remove_bomb(game_state, bomb_exploded);
//
//    set_dead_players(game_state);
//
//    remove_blocks(game_state);
//
//    if (!is_explosion_inside_a_block(game_state, bomb_position)) {
//        add_explosions(game_state, bomb_position);
//    }
//}
//
//void PlayerMovedEvent::execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) {
//    game_state.player_positions.get_map()[id] = Position(position.get_x().get_value(), position.get_y().get_value());
//}
//
//void BlockPlacedEvent::execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) {
//    game_state.blocks.get_list().push_back(position);
//}
//
//Event::event_t Event::get_event(Bytes &bytes) {
//    type = (Type) bytes.get_next_byte();
//    switch(type) {
//        case BombPlaced:
//            return BombPlacedEvent(bytes);
//            break;
//        case BombExploded:
//            return BombExplodedEvent(bytes);
//            break;
//        case PlayerMoved:
//            return PlayerMovedEvent(bytes);
//            break;
//        case BlockPlaced:
//            return BlockPlacedEvent(bytes);
//            break;
//        default:
//            throw BytesDeserializationException();
//    }
//}
//
//void Event::execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) {
//    switch(type) {
//        case BombPlaced:
//            std::get<BombPlacedEvent>(event).execute(game_state, sockets_info);
//            break;
//        case BombExploded:
//            std::get<BombExplodedEvent>(event).execute(game_state, sockets_info);
//            break;
//        case PlayerMoved:
//            std::get<PlayerMovedEvent>(event).execute(game_state, sockets_info);
//            break;
//        case BlockPlaced:
//            std::get<BlockPlacedEvent>(event).execute(game_state, sockets_info);
//            break;
//    }
//}
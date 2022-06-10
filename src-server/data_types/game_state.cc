#include "game_state.h"
#include "../messages.h"

void GameState::try_add_player(const String &player_name, const String &address) {
    std::unique_lock<std::mutex> lock(mutex);

    if (players.get_map().size() == players_count.get_value()) {
        return;
    }

    auto player = Player(player_name, address);
    for (auto &[key, value] : players.get_map()) {
        if (value == player) {
            return;
        }
    }

    players.get_map()[next_player_id] = player;
    auto id = player_id_t(next_player_id);
    accepted_players_to_send.push_back(std::make_shared<AcceptedPlayerMessage>(id, player));
    next_player_id++;

    if (players.get_map().size() == players_count.get_value()) {
        is_started = true;
    }
}

void GameState::start_game() {
    PointerList<Event> events;

    for (uint8_t id = 0; id < players_count.get_value(); id++) {
        auto position = get_random_position();

        player_positions.get_map()[id] = position;
        auto id_temp = player_id_t(id);

        auto event = std::make_shared<PlayerMovedEvent>(id_temp, position);
        events.get_list().push_back(event);
    }

    for (uint16_t i = 0; i < initial_blocks; i++) {
        auto position = get_random_position();

        if (!blocks.get_set().contains(position)) {
            blocks.get_set().insert(position);
            auto event = std::make_shared<BlockPlacedEvent>(position);
            events.get_list().push_back(event);
        }
    }

    turn_messages.push_back(std::make_shared<TurnMessage>(turn, events));
}

void GameState::add_explosion(const Position &bomb_position, List<player_id_t> &robots_destroyed,
                              List<Position> &blocks_destroyed) {
    for (auto &[id, position] : player_positions.get_map()) {
        if (bomb_position == position) {
            robots_destroyed.get_list().push_back(id);
            player_deaths_this_round.insert(id);
        }
    }

    for (auto &block_position : blocks.get_set()) {
        if (bomb_position == block_position) {
            blocks_destroyed.get_list().push_back(bomb_position);
            blocks_destroyed_this_round.insert(block_position);

            return;
        }
    }
    for (size_t i = 0; i < 4; i++) {
        auto direction = static_cast<Direction>(i);
        auto current_position = bomb_position;
        bool do_continue = true;

        for (size_t r = 0; r < explosion_radius.get_value()
                && do_continue && current_position.is_next_proper(direction, size_x, size_y); r++) {

            current_position = current_position.next(direction);

            for (auto &[id, position] : player_positions.get_map()) {
                if (current_position == position) {
                    robots_destroyed.get_list().push_back(id);
                    player_deaths_this_round.insert(id);
                }
            }

            for (auto &block_position : blocks.get_set()) {
                if (current_position == block_position) {
                    do_continue = false;
                    blocks_destroyed.get_list().push_back(current_position);
                    blocks_destroyed_this_round.insert(block_position);
                    break;
                }
            }
        }
    }
}

void GameState::next_turn() {
//    std::cout << "next turn" << std::endl;
    player_deaths_this_round.clear();
    blocks_destroyed_this_round.clear();

    if (is_started) {
        if (turn == 0) {
            start_game();
            turn += 1;
            is_first_turn = true;
            return;
        }
        else if (turn == 1) {
            is_first_turn = false;
        }
        PointerList<Event> events;

        std::set<bomb_id_t> bombs_to_remove;

        for (auto &[id, bomb] : bombs_map) {
            bomb->decrease_timer();
            if (bomb->does_explode()) {
                auto position = bomb->get_position();
                List<player_id_t> robots_destroyed;
                List<Position> blocks_destroyed;

                add_explosion(position, robots_destroyed, blocks_destroyed);

                events.get_list().push_back(std::make_shared<BombExplodedEvent>(id, robots_destroyed, blocks_destroyed));

                bombs_to_remove.insert(id);
                auto bombs_it = bombs.get_list().begin();
                while (*bombs_it != bomb) {
                    bombs_it++;
                }
                bombs.get_list().erase(bombs_it);
            }
        }

        for (auto &id : bombs_to_remove) {
            bombs_map.erase(id);
        }

        for (auto &position : blocks_destroyed_this_round) {
            blocks.get_set().erase(position);
        }

        for (uint8_t id = 0; id < players_count.get_value(); id++) {
//            std::cout << "PLAYER ID = " << (int) id << std::endl;
            if (player_deaths_this_round.contains(id)) {
                auto position = get_random_position();
                player_positions.get_map()[id] = position;
                auto id_temp = player_id_t(id);
                events.get_list().push_back(std::make_shared<PlayerMovedEvent>(id_temp, position));
                scores.get_map()[id] += 1;
            }
            else {
                if (players_action.contains(id)) {
                    auto action = players_action[id];
                    if (action) {
                        auto event = action->execute(this, id);
                        if (event != nullptr) {
                            events.get_list().push_back(event);
                        }
                        players_action.erase(id);
                    }
                }
            }
        }

        turn_messages.push_back(std::make_shared<TurnMessage>(turn, events));

        if (turn.get_value() % 1000 == 0) {
            std::cerr << turn.get_value() << std::endl;
        }

        if (turn == game_length.get_value()) {
            is_ended = true;
            return;
        }

        turn += 1;
    }

//    std::cout << "next turn end" << std::endl;
}

void GameState::send_to_all() {
//    std::cout << "send_to_all" << std::endl;

    std::set<std::shared_ptr<ClientState>> to_remove;
    for (auto &client : clients) {
        if (client->get_ended()) {
            to_remove.insert(client);
            continue;
        }

        try {
            auto messages = get_messages_to_send(*client);
            for (auto &message: messages) {
    //                std::cout << "sending..." << std::endl;

                    message->send(client->get_socket());
    //                std::cout << "sent." << std::endl;
            }
        }
        catch (std::exception &exception) {
            std::cerr << exception.what() << std::endl;
            client->end_threads();
            to_remove.insert(client);
        }
    }

    for (auto &client : to_remove) {
        clients.erase(client);
    }

    if (is_ended) {
        game_number++;
        reset();
    }

//    std::cout << "send_to_all end" << std::endl;
}

std::vector<std::shared_ptr<ServerMessage>> GameState::get_messages_to_send(ClientState &client_state) {
    std::vector<std::shared_ptr<ServerMessage>> messages;

    if (!is_started || is_first_turn) {
        auto accepted_players_iterator = accepted_players_to_send.begin() + client_state.get_accepted_players_sent();
        while (accepted_players_iterator != accepted_players_to_send.end()) {
            messages.push_back(*accepted_players_iterator);
            accepted_players_iterator++;
            client_state.increase_accepted_players_sent(1);
        }
    }

    if (is_started && !client_state.get_game_started_sent()) {
        messages.push_back(std::make_shared<GameStartedMessage>(players));
        client_state.set_game_started_sent();
    }

    auto turn_messages_iterator = turn_messages.begin() + client_state.get_turns_sent();
    while (turn_messages_iterator != turn_messages.end()) {
        messages.push_back(*turn_messages_iterator);
        turn_messages_iterator++;
        client_state.increase_turns_sent(1);
    }

    if (is_ended && !client_state.get_game_ended_sent()) {
        messages.push_back(std::make_shared<GameEndedMessage>(scores));
        client_state.set_game_ended_sent();
    }

    return messages;
}

void GameState::next_loop() {
    std::unique_lock<std::mutex> lock(mutex);
    main_loop.wait_for(lock, std::chrono::milliseconds(turn_duration));
    next_turn();
    lock.unlock();
    send_to_all();
}
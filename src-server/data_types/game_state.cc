#include "game_state.h"
#include "../messages.h"

void GameState::try_add_player(const String &player_name, const String &address) {
    std::unique_lock<std::mutex> lock(mutex);
    while (is_sending) { // TODO
        sending_condition.wait(lock);
    }

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

    if (players.get_map().size() == players_count) {
        is_started = true;
        start_game();
    }
}

void GameState::start_game() {
    turn = 0;
    PointerList<Event> events;

    for (uint8_t id = 0; id < players_count.get_value(); id++) {
        auto position = Position(random() % size_x.get_value(), random() % size_y.get_value());
        player_positions.get_map()[id] = position;
        auto id_temp = player_id_t(id);
        auto event = std::make_shared<PlayerMovedEvent>(id_temp, position);
        events.get_list().push_back(event);
    }

    turn_messages.push_back(std::make_shared<TurnMessage>(turn, events));
}

void GameState::next_turn() {
    std::unique_lock<std::mutex> lock(mutex);

    std::cout << "next turn" << std::endl;

    if (is_started) {
        turn += 1;
        PointerList<Event> events;

        for (uint8_t id = 0; id < players_count.get_value(); id++) {
            if (players_action.contains(id)) {
                events.get_list().push_back(players_action[id]->execute(this, id));
                players_action.erase(id);
            }
        }

        turn_messages.push_back(std::make_shared<TurnMessage>(turn, events));
    }

    std::cout << "next turn end" << std::endl;
}

void GameState::send_next() {
//    std::cout << "send_next" << std::endl;

    is_sending = true;

    {
        std::unique_lock<std::mutex> lock(mutex);
    }
    sending_condition.notify_all();

    {
        std::unique_lock<std::mutex> lock(mutex);
        while (how_many_to_send != 0) {
            sending_ended.wait(lock);
        }

        is_sending = false;
        sending_ended.notify_all();
    }

//    std::cout << "send_next end" << std::endl;
}

std::vector<std::shared_ptr<ServerMessage>> GameState::get_messages(ClientState &client_state) {
    if (is_sending) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (is_sending) {
                sending_ended.wait(lock);
            }
        }
    }
    sending_ended.notify_all();

    how_many_to_send++;
//    std::cout << "get_messages" << std::endl;
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (!is_sending) {
//            std::cout << "sending_condition.wait" << std::endl;
            sending_condition.wait(lock);
        }
    }
    sending_condition.notify_all();

    std::cout << "messages" << std::endl;
    std::vector<std::shared_ptr<ServerMessage>> messages;

    auto accepted_players_iterator = accepted_players_to_send.begin() + client_state.get_accepted_players_sent();
    std::cout << "accepted players sent: " << client_state.get_accepted_players_sent() << std::endl;
    while (accepted_players_iterator != accepted_players_to_send.end()) {
        std::cout << "accepted player to send" << std::endl;
        messages.push_back(*accepted_players_iterator);
        accepted_players_iterator++;
        client_state.increase_accepted_players_sent(1);
    }

    std::cout << "is_started: " << is_started << std::endl;
    if (is_started && !client_state.get_game_started_sent()) {
        messages.push_back(std::make_shared<GameStartedMessage>(players));
        client_state.set_game_started_sent();
    }

    auto turn_messages_iterator = turn_messages.begin() + client_state.get_turns_sent();
    std::cout << "turns sent: " << client_state.get_turns_sent() << std::endl;
    while (turn_messages_iterator != turn_messages.end()) {
        std::cout << "turn message to send" << std::endl;
        messages.push_back(*turn_messages_iterator);
        turn_messages_iterator++;
        client_state.increase_turns_sent(1);
    }

    how_many_to_send--;
    if (how_many_to_send == 0) {
        sending_ended.notify_all();
    }

    std::cout << "get_messages end" << std::endl;

    return messages;
}
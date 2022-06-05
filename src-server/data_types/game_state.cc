#include "game_state.h"
#include "../messages.h"

void GameState::try_add_player(const String &player_name, const String &address) {
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
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
    }
    is_sending = false;
    sending_ended.notify_all();

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
    if (is_started) {

    }
    else {
        auto it = accepted_players_to_send.begin() + client_state.get_accepted_players_sent();
        std::cout << "accepted players sent: " << client_state.get_accepted_players_sent() << std::endl;
        while (it != accepted_players_to_send.end()) {
            std::cout << "accepted player to send" << std::endl;
            messages.push_back(*it);
            it++;
            client_state.increase_accepted_players_sent(1);
        }
//        for (auto id : accepted_players_to_send) {
//            auto id_temp = player_id_t(id);
//            auto player = players.get_map()[id];
//            messages.push_back(std::make_shared<AcceptedPlayerMessage>(id_temp, player));
//        }
    }

    how_many_to_send--;
    if (how_many_to_send == 0) {
        sending_ended.notify_all();
    }

    std::cout << "get_messages end" << std::endl;

    return messages;
}
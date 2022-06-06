#include "threads.h"
#include "messages.h"

void sender_fun(std::shared_ptr<ClientInfo> client_info, std::shared_ptr<GameState> &game_state) {
    ClientState client_state;

    try {
        {
            std::unique_lock<std::mutex> lock(client_info->get_mutex());
            while (!client_info->get_threads_set()) {
                client_info->get_condition().wait(lock);
            }
        }

        HelloMessage(game_state).send(client_info->get_socket());

        for (;;) {
            if (client_info->get_ended()) {
//                std::cout << "return" << std::endl;
//                client_info->end_threads(current_connections);
                return;
            }

            auto messages = game_state->get_messages(client_state);
            for (auto &message: messages) {
                message->send(client_info->get_socket());
            }
            if (client_state.get_game_ended_sent()) {
                return;
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        client_info->end_threads();
    }
}

void receiver_fun(std::shared_ptr<ClientInfo> client_info, std::shared_ptr<GameState> &game_state,
                  std::atomic_int &current_connections) {
    try {
        {
            std::unique_lock<std::mutex> lock(client_info->get_mutex());
            while (!client_info->get_threads_set()) {
                client_info->get_condition().wait(lock);
            }
        }
        for (;;) {
            if (client_info->get_ended()) {
//                std::cout << "return" << std::endl;
//                client_info->end_threads(current_connections);
                current_connections--;
                return;
            }
            BytesReceiver bytes(client_info->get_socket());
            while (!bytes.is_end()) {
                auto message = get_client_message(bytes);
                message->execute(game_state, client_info);
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        client_info->end_threads();
        current_connections--;
    }
}
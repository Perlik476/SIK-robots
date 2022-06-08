#include "threads.h"
#include "messages.h"

void acceptor_fun(std::shared_ptr<GameState> &game_state, boost::asio::io_context &io_context,
                  tcp::acceptor &acceptor) {
    try {
        std::atomic_int current_connections = 0;
        const int max_connections = 25;
        for (;;) {
            if (current_connections < max_connections) {
                auto socket = std::make_shared<tcp::socket>(io_context);
                acceptor.accept(*socket);
                tcp::no_delay option(true);
                socket->set_option(option);

                current_connections++;

                std::cout << "Accepted" << std::endl;

                auto client_info = std::make_shared<ClientInfo>(socket);

                auto receiver_thread = std::make_shared<std::thread>(receiver_fun, client_info, std::ref(game_state),
                                                                     std::ref(current_connections));
                receiver_thread->detach();

                auto sender_thread = std::make_shared<std::thread>(sender_fun, client_info, std::ref(game_state));
                sender_thread->detach();

                client_info->set_sender();
                client_info->set_receiver();

                std::cout << "connections: " << (int) current_connections<< std::endl;
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }
}

void main_loop(std::shared_ptr<GameState> &game_state, boost::asio::io_context &io_context) {
    for (;;) {
        boost::asio::steady_timer timer(io_context, boost::asio::chrono::milliseconds(game_state->get_turn_duration()));
        timer.wait();
        game_state->next_turn();
        game_state->send_next();
    }
}

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

            auto messages = game_state->get_messages_to_send(client_state);
            for (auto &message: messages) {
                message->send(client_info->get_socket());
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
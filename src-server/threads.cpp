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

//                std::cout << "Accepted" << std::endl;

                auto client = std::make_shared<ClientState>(socket);
                game_state->add_client(client);
                try {
                    HelloMessage(game_state).send(client->get_socket());
                }
                catch (...) {
                    std::cerr << "Hello error" << std::endl; // TODO
                }
//                game_state

                auto receiver_thread = std::make_shared<std::thread>(receiver_fun, client, std::ref(game_state),
                                                                     std::ref(current_connections));
                receiver_thread->detach();

//                std::cout << "connections: " << (int) current_connections<< std::endl;
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }
}

void main_loop(std::shared_ptr<GameState> &game_state) {
    for (;;) {
        game_state->next_loop();
    }
}
//
//void sender_fun(std::shared_ptr<ClientState> client_info, std::shared_ptr<GameState> &game_state) {
//    ClientState client_state;
//
//    try {
//        for (;;) {
//            if (client_info->get_ended()) {
//                return;
//            }
//
//            auto messages = game_state->get_messages_to_send(client_state);
//            for (auto &message: messages) {
////                std::cout << "sending..." << std::endl;
//                message->send(client_info->get_socket());
////                std::cout << "sent." << std::endl;
//            }
//        }
//    }
//    catch (std::exception &exception) {
//        std::cerr << exception.what() << std::endl;
//        client_info->end_threads();
//    }
//}

void receiver_fun(std::shared_ptr<ClientState> client, std::shared_ptr<GameState> &game_state,
                  std::atomic_int &current_connections) {
    try {
        for (;;) {
            if (client->get_ended()) {
                current_connections--;
                return;
            }
            BytesReceiver bytes(client->get_socket());
            while (!bytes.is_end()) {
                auto message = get_client_message(bytes);
                message->execute(game_state, client);
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        client->end_threads();
        current_connections--;
    }
}
#include "threads.h"
#include "messages.h"

void acceptor_fun(std::shared_ptr<GameState> &game_state, boost::asio::io_context &io_context,
                  tcp::acceptor &acceptor) {
    try {
        std::atomic_int current_connections = 0;
        const int max_connections = 25;
        for (;;) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            tcp::no_delay option(true);
            socket->set_option(option);

            if (current_connections == max_connections) {
                socket->close();
                continue;
            }

            current_connections++;

            auto client = std::make_shared<ClientState>(socket);
            game_state->add_client(client);
            try {
                HelloMessage(game_state).send(client->get_socket());
            }
            catch (std::exception &exception) {
                std::cerr << "Connection error: " << exception.what() << std::endl;
                continue;
            }

            auto receiver_thread = std::make_shared<std::thread>(receiver_fun, client, std::ref(game_state),
                                                                 std::ref(current_connections));
            receiver_thread->detach();
        }
    }
    catch (std::exception &exception) {
        std::cerr << "Acceptor failed: " << exception.what() << std::endl;
    }
}

void game_loop(std::shared_ptr<GameState> &game_state) {
    for (;;) {
        game_state->next_loop();
    }
}

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
        if (!client->get_ended())
            std::cerr << "Connection error: " << exception.what() << std::endl;
        client->end_threads();
        current_connections--;
    }
}
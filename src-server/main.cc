#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <thread>

#include "includes.h"
#include "messages.h"
#include "threads.h"

void acceptor_fun(std::shared_ptr<Arguments> &arguments, std::shared_ptr<GameState> &game_state,
                  boost::asio::io_context &io_context, tcp::acceptor &acceptor) {
    try {
        std::atomic_int current_connections = 0;
        const int max_connections = 25;
        for (;;) {
            if (current_connections < max_connections) {
                auto socket = std::make_shared<tcp::socket>(io_context);
                acceptor.accept(*socket);

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

int main(int argc, char *argv[]) {
    std::shared_ptr<Arguments> arguments;

    try {
        arguments = std::make_shared<Arguments>(argc, argv);
    }
    catch (std::exception &exception) {
        std::cerr << "Program arguments are invalid. Terminating." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }
    if (!arguments->check_correctness()) {
        std::cerr << "Program arguments are invalid. Terminating." << std::endl;
        return 1;
    }

    auto game_state = std::make_shared<GameState>(arguments);
    boost::asio::io_context io_context;

    try {
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v6(), arguments->get_port()));

        std::thread main_thread{main_loop, std::ref(game_state), std::ref(io_context)};
        std::thread acceptor_thread{acceptor_fun, std::ref(arguments), std::ref(game_state), std::ref(io_context),
                                    std::ref(acceptor)};

        main_thread.detach();

        acceptor_thread.join();
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 1;
}
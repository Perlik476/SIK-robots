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
                  boost::asio::io_context &io_context) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v6(), arguments->get_port()));

//    threads_t senders = std::make_shared<std::set<thread_t>>();
//    threads_t receivers = std::make_shared<std::set<thread_t>>();

    std::atomic_int current_connections = 0;
    const int max_connections = 25;
    try {
        for (;;) {
            if (current_connections < max_connections) {
                auto socket = std::make_shared<tcp::socket>(io_context);
                acceptor.accept(*socket);

                current_connections++;

                std::cout << "Accepted" << std::endl;

                auto client_info = std::make_shared<ClientInfo>(socket);

                auto receiver_thread = std::make_shared<std::thread>(receiver_fun, client_info, std::ref(game_state),
                                                                     std::ref(current_connections));

                // TODO
                receiver_thread->detach();

//                senders->insert(sender_thread);
//                receivers->insert(receiver_thread);

                client_info->set_sender();
                client_info->set_receiver();

                std::cout << "connections: " << (int) current_connections<< std::endl;
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << "wtf xd" << std::endl;
        std::cerr << exception.what() << std::endl;
    }

    std::cout << "exit" << std::endl;
//    for (auto &sender : *senders) {
//        sender->join();
//    }
//    for (auto &receiver : *receivers) {
//        receiver->join();
//    }
}

void main_loop(std::shared_ptr<GameState> &game_state, boost::asio::io_context &io_context) {
    for (;;) {
        boost::asio::steady_timer timer(io_context, boost::asio::chrono::milliseconds(game_state->get_turn_duration()));
        timer.wait();
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

    std::cout << "OK\n";

    auto game_state = std::make_shared<GameState>(arguments);
    boost::asio::io_context io_context;

    std::thread main_thread{main_loop, std::ref(game_state), std::ref(io_context)};
    std::thread acceptor_thread{acceptor_fun, std::ref(arguments), std::ref(game_state), std::ref(io_context)};

    acceptor_thread.join();
    main_thread.join();

    return 1;
}
#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <ranges>
#include <thread>

#include "includes.h"
#include "messages.h"
#include "threads.h"

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

    auto game_state = std::make_shared<GameState>(arguments);
    boost::asio::io_context io_context;

    try {
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v6(), arguments->get_port()));

        std::thread main_thread{main_loop, std::ref(game_state)};
        std::thread acceptor_thread{acceptor_fun, std::ref(game_state), std::ref(io_context), std::ref(acceptor)};

        main_thread.detach();

        acceptor_thread.join();
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 1;
}
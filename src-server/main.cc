#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <thread>

#include "includes.h"
#include "messages.h"

using thread_t = std::shared_ptr<std::thread>;
using threads_t = std::shared_ptr<std::set<thread_t>>;

class ThreadInfo {
    threads_t senders;
    threads_t receivers;
    thread_t sender;
    thread_t receiver;
    socket_t socket;

public:
    ThreadInfo(threads_t &senders, threads_t &receivers, thread_t &sender, thread_t &receiver, socket_t &socket)
        : senders(senders), receivers(receivers), sender(sender), receiver(receiver), socket(socket) {}

    void end_threads() {
        senders->erase(sender);
        receivers->erase(receiver);

        socket->close();
    }

    socket_t &get_socket() { return socket; }
};

void sender_fun(socket_t socket, std::shared_ptr<GameState> &game_state) {
    for (;;) {
        auto messages = game_state->get_messages();
        for (auto &message : messages) {
            message->send(socket);
        }
    }
}

void receiver_fun(socket_t socket, std::shared_ptr<GameState> &game_state) {
    for (;;) {
        BytesReceiver bytes(socket);
        while (!bytes.is_end()) {
            auto message = get_client_message(bytes);
            message->execute(game_state, socket);
        }
    }
}

void acceptor_fun(std::shared_ptr<Arguments> &arguments, std::shared_ptr<GameState> &game_state,
                  boost::asio::io_context &io_context) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v6(), arguments->get_port()));

    threads_t senders = std::make_shared<std::set<thread_t>>();
    threads_t receivers = std::make_shared<std::set<thread_t>>();

    uint8_t current_connections = 0;
    const uint8_t max_connections = 25;
    for (;;) {
        if (current_connections < max_connections) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            current_connections++;

            std::cout << "Accepted" << std::endl;

            auto sender_thread = std::make_shared<std::thread>(sender_fun, socket, std::ref(game_state));
            auto receiver_thread = std::make_shared<std::thread>(receiver_fun, socket, std::ref(game_state));

            senders->insert(sender_thread);
            receivers->insert(receiver_thread);

            std::cout << "connections: " << current_connections << " | senders: " << senders->size()
                << " | receivers: " << receivers->size() << std::endl;
        }
    }

    for (auto &sender : *senders) {
        sender->join();
    }
    for (auto &receiver : *receivers) {
        receiver->join();
    }
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

//    ThreadsInfo threads_info;
//    std::shared_ptr<SocketsInfo> sockets_info;
//    try {
//        sockets_info = std::make_shared<SocketsInfo>(*arguments);
//    }
//    catch (std::exception &exception) {
//        std::cerr << exception.what() << std::endl;
//        return 1;
//    }
//
//    std::thread gui_listener_thread{from_gui_to_server_communication, std::ref(game_state),
//                                    std::ref(*sockets_info), std::ref(threads_info)};
//    std::thread server_listener_thread{from_server_to_gui_communication, std::ref(game_state),
//                                       std::ref(*sockets_info), std::ref(threads_info)};
//
//    {
//        std::unique_lock<std::mutex> lock(threads_info.get_mutex());
//        while (!threads_info.get_should_exit()) {
//            threads_info.get_condition_variable().wait(lock);
//        }
//        std::cerr << "Exception occurred in one of the threads." << std::endl;
//    }
//
//    boost::system::error_code error_code;
//
//    sockets_info->get_gui_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
//    sockets_info->get_gui_socket()->close(error_code);
//
//    sockets_info->get_server_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
//    sockets_info->get_server_socket()->close(error_code);
//
//    gui_listener_thread.join();
//    server_listener_thread.join();
//
//    std::cerr << "Client terminating." << std::endl;

    return 1;
}
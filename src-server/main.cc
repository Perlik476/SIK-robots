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

void sender_fun(socket_t &socket) {
    for (int i = 0; i < 100000; i++) {

    }
}

void receiver_fun(socket_t &socket) {
    for (;;) {
        BytesReceiver bytes(socket);
        while (!bytes.is_end()) {
            bytes.get_next_byte();
        }
    }
}

void acceptor_fun(std::shared_ptr<Arguments> &arguments) {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v6(), arguments->get_port()));

    threads_t senders = std::make_shared<std::set<thread_t>>();
    threads_t receivers = std::make_shared<std::set<thread_t>>();

    for (;;) {
        auto socket = std::make_shared<tcp::socket>(io_context);
        acceptor.accept(*socket);

        std::cout << "Accepted" << std::endl;

        auto sender_thread = std::make_shared<std::thread>(sender_fun, std::ref(socket));
        auto receiver_thread = std::make_shared<std::thread>(receiver_fun, std::ref(socket));

        senders->insert(sender_thread);
        receivers->insert(receiver_thread);

        std::cout << "senders: " << senders->size() << " | receivers: " << receivers->size() << std::endl;
    }

    for (auto &sender : *senders) {
        sender->join();
    }
    for (auto &receiver : *receivers) {
        receiver->join();
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

    std::thread acceptor_thread{acceptor_fun, std::ref(arguments)};

    acceptor_thread.join();

    auto game_state = std::make_shared<GameState>(arguments);

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
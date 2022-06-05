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

class ClientInfo {
//    threads_t senders;
//    threads_t receivers;
//    thread_t sender;
//    thread_t receiver;
    socket_t socket;
    std::atomic_bool ended = false;
    bool threads_set = false;
    std::mutex mutex;
    std::condition_variable condition;
    bool sender = false;
    bool receiver = false;

    void check_threads_set() {
        if (sender && receiver) {
            threads_set = true;
            condition.notify_all();
        }
    }

public:
    ClientInfo(socket_t &socket) : socket(socket) {}

    void end_threads(std::atomic_int &current_connections) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!ended) {
            ended = true;
            current_connections--;

            boost::system::error_code err;
            socket->close(err);
        }
    }

    void set_sender() {
        std::unique_lock<std::mutex> lock(mutex);
        sender = true;
        check_threads_set();
    }

    void set_receiver() {
        std::unique_lock<std::mutex> lock(mutex);
        receiver = true;
        check_threads_set();
    }

    bool get_threads_set() {
        return threads_set;
    }

    std::mutex &get_mutex() {
        return mutex;
    }

    std::condition_variable &get_condition() {
        return condition;
    }

    socket_t &get_socket() {
        return socket;
    }

    bool get_ended() {
        return ended;
    }
};

void sender_fun(std::shared_ptr<ClientInfo> client_info, std::shared_ptr<GameState> &game_state,
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
                return;
            }
            auto messages = game_state->get_messages();
            for (auto &message: messages) {
                message->send(client_info->get_socket());
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        client_info->end_threads(current_connections);
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
                return;
            }
            BytesReceiver bytes(client_info->get_socket());
            while (!bytes.is_end()) {
                auto message = get_client_message(bytes);
                message->execute(game_state, client_info->get_socket());
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        client_info->end_threads(current_connections);
    }
}

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

                auto client = std::make_shared<ClientInfo>(socket);

                auto sender_thread = std::make_shared<std::thread>(sender_fun, client, std::ref(game_state), std::ref(current_connections));
                auto receiver_thread = std::make_shared<std::thread>(receiver_fun, client, std::ref(game_state), std::ref(current_connections));

                // TODO
                sender_thread->detach();
                receiver_thread->detach();

//                senders->insert(sender_thread);
//                receivers->insert(receiver_thread);

                client->set_sender();
                client->set_receiver();

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
#ifndef ROBOTS_CLIENT_MESSAGES_H
#define ROBOTS_CLIENT_MESSAGES_H

#include "../includes.h"
#include "server_messages.h"

enum ClientMessageType: char {
    Join,
    PlaceBomb,
    PlaceBlock,
    Move,
};

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

    void end_threads() {
        std::unique_lock<std::mutex> lock(mutex);
        if (!ended) {
            ended = true;

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

class ClientMessage : public Executable {};

class JoinMessage : public ClientMessage {
private:
    String name;


public:
    JoinMessage(Bytes &bytes) : name(bytes) {}

    void execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientInfo> &client_info);
};

class PlaceBombMessage : public ClientMessage {
public:
    explicit PlaceBombMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] std::shared_ptr<ClientInfo> &client_info) override {
        std::cout << "PlaceBomb" << std::endl;
    }
};


class PlaceBlockMessage : public ClientMessage {
public:
    explicit PlaceBlockMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] std::shared_ptr<ClientInfo> &client_info) override {
        std::cout << "PlaceBlock" << std::endl;
    }
};

class MoveMessage : public ClientMessage {
private:
    Direction direction = Undefined;

public:
    MoveMessage(Bytes &bytes) : direction(get_direction(bytes)) {}

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] std::shared_ptr<ClientInfo> &client_info) override {
        std::cout << "Move: " << direction << std::endl;
    }
};

#endif //ROBOTS_CLIENT_MESSAGES_H

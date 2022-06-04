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

class ClientMessage : public Executable {};

class JoinMessage : public ClientMessage {
private:
    String name;

public:
    JoinMessage(Bytes &bytes) : name(bytes) {}

    void execute(std::shared_ptr<GameState> &game_state, socket_t &socket) override {
        std::cout << "Join: " << name.get_string() << std::endl;
        HelloMessage(game_state).send(socket);
    }
};

class PlaceBombMessage : public ClientMessage {
public:
    explicit PlaceBombMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] socket_t &socket) override {
        std::cout << "PlaceBomb" << std::endl;
    }
};


class PlaceBlockMessage : public ClientMessage {
public:
    explicit PlaceBlockMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] socket_t &socket) override {
        std::cout << "PlaceBlock" << std::endl;
    }
};

class MoveMessage : public ClientMessage {
private:
    Direction direction = Undefined;

public:
    MoveMessage(Bytes &bytes) : direction(get_direction(bytes)) {}

    void execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] socket_t &socket) override {
        std::cout << "Move: " << direction << std::endl;
    }
};

#endif //ROBOTS_CLIENT_MESSAGES_H

#ifndef ROBOTS_CLIENT_MESSAGES_H
#define ROBOTS_CLIENT_MESSAGES_H

#include "../includes.h"
#include "server_messages.h"
#include "../data_types/definitions.h"

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

    void execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info);
};

class PlaceBombMessage : public ClientMessage {
public:
    explicit PlaceBombMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) override;
};


class PlaceBlockMessage : public ClientMessage {
public:
    explicit PlaceBlockMessage() = default;

    void execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) override;
};

class MoveMessage : public ClientMessage {
private:
    Direction direction = Undefined;

public:
    MoveMessage(Bytes &bytes) : direction(get_direction(bytes)) {}

    void execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) override;
};

#endif //ROBOTS_CLIENT_MESSAGES_H

#ifndef ROBOTS_GUI_MESSAGES_H
#define ROBOTS_GUI_MESSAGES_H

#include "../includes.h"
#include "client_messages.h"

enum GuiMessageType: char {
    PlaceBomb,
    PlaceBlock,
    Move
};

class GuiMessage: public Executable {};

class PlaceBombGuiMessage: public GuiMessage {
public:
    PlaceBombGuiMessage() = default;

    void execute([[maybe_unused]] GameState &game_state, SocketsInfo &sockets_info) override {
        PlaceBombMessage().send(sockets_info.get_server_socket());
    }
};

class PlaceBlockGuiMessage: public GuiMessage {
public:
    PlaceBlockGuiMessage() = default;

    void execute([[maybe_unused]] GameState &game_state, SocketsInfo &sockets_info) override {
        PlaceBlockMessage().send(sockets_info.get_server_socket());
    }
};

class MoveGuiMessage: public GuiMessage {
    Direction direction;
public:
    explicit MoveGuiMessage(Bytes &bytes) : direction(get_direction(bytes)) {}

    void execute([[maybe_unused]] GameState &game_state, SocketsInfo &sockets_info) override {
        MoveMessage(direction).send(sockets_info.get_server_socket());
    }
};

#endif //ROBOTS_GUI_MESSAGES_H

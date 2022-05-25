#ifndef ROBOTS_GUI_MESSAGES_H
#define ROBOTS_GUI_MESSAGES_H

#include "../definitions.h"
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
        std::cout << "PlaceBomb sent." << std::endl;
        PlaceBombMessage().send(sockets_info.get_server_socket());
    }
};

class PlaceBlockGuiMessage: public GuiMessage {
public:
    PlaceBlockGuiMessage() = default;

    void execute([[maybe_unused]] GameState &game_state, SocketsInfo &sockets_info) override {
        std::cout << "PlaceBlock sent." << std::endl;
        PlaceBlockMessage().send(sockets_info.get_server_socket());
    }
};

class MoveGuiMessage: public GuiMessage {
    Direction direction;
public:
    explicit MoveGuiMessage(Bytes &bytes) : direction(get_direction(bytes)) {}

    void execute([[maybe_unused]] GameState &game_state, SocketsInfo &sockets_info) override {
        std::cout << "Move sent:" << (int) direction << std::endl;
        MoveMessage(direction).send(sockets_info.get_server_socket());\
    }
};

#endif //ROBOTS_GUI_MESSAGES_H

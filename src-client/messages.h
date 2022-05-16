#ifndef ROBOTS_MESSAGES_H
#define ROBOTS_MESSAGES_H

#include "definitions.h"

class MessageToSend: public Serializable {
public:
    virtual void send() = 0;
};

class ClientMessage: public MessageToSend {
private:
    virtual char get_identifier() = 0;
public:
    void send() override {
        Bytes message = serialize();
    }

    Bytes serialize() override {
        return {get_identifier()};
    }
};

class JoinMessage : public ClientMessage {
private:
    char get_identifier() override { return 0; }
    String name;

public:
    JoinMessage() = default;

    explicit JoinMessage(const std::string &name): name(String(name)) {}

    explicit JoinMessage(const String &name): name(name) {}

    Bytes serialize() override {
        return Bytes(get_identifier()) + name.serialize();
    }
};

class PlaceBombMessage : public ClientMessage {
private:
    char get_identifier() override { return 1; }

public:
    PlaceBombMessage() = default;
};

class PlaceBlockMessage : public ClientMessage {
private:
    char get_identifier() override { return 2; }

public:
    PlaceBlockMessage() = default;
};

class MoveMessage : public ClientMessage {
private:
    char get_identifier() override { return 3; }
    Direction direction = Undefined;

public:
    MoveMessage() = default;

    explicit MoveMessage(const Direction direction): direction(direction) {}

    Bytes serialize() override {
        return Bytes(get_identifier()) + Bytes(direction);
    }
};


class DrawMessage: public MessageToSend {
private:
    virtual char get_identifier() = 0;
public:
    void send() override {
        Bytes message = serialize();
    }
};

class LobbyMessage : public DrawMessage {
private:
//    char get_identifier() override { return 0; }
//    String _server_name;
//    Uint<uint8_t> _players_count;
//    Uint<uint16_t> _size_x;
//    Uint<uint16_t> _size_t;
//    Uint<uint16_t> _game_length;
//    Uint<uint16_t> _explosion_radius;
//    Uint<uint16_t> _bomb_timer;
//    Map<PlayerId, Player> _players;
    Bytes message;

public:
    LobbyMessage() = default;

    LobbyMessage(Bytes message) : message(message) {}

    Bytes serialize() override { return message; }
};


class MessageReceived {
    Bytes bytes;
    virtual void deserialize() = 0;
public:
    MessageReceived() = default;

    explicit MessageReceived(Bytes &bytes) : bytes(bytes) {}

    virtual void execute(GameState &game_state) = 0;
};

class ServerMessage: public MessageReceived {};

//class HelloMessage: public ServerMessage {
//    void deserialize() override {
//
//    }
//public:
//    void execute(GameState &game_state) override {
//
//    }
//};

class GuiMessage: public MessageReceived {};

std::shared_ptr<ServerMessage> get_server_message(Bytes bytes);

std::shared_ptr<GuiMessage> get_gui_message(Bytes bytes);

#endif // ROBOTS_MESSAGES_H
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


//class MessageReceived {
//public:
//    MessageReceived() = default;
//
//    virtual void execute(GameState &game_state) = 0;
//};

class ServerMessage: public Executable {};

class HelloMessage: public ServerMessage {
    String server_name;
    Uint8 players_count;
    Uint16 size_x;
    Uint16 size_y;
    Uint16 game_length;
    Uint16 explosion_radius;
    Uint16 bomb_timer;

public:
    explicit HelloMessage(Bytes &bytes) {
        server_name = String(bytes);
        players_count = Uint8(bytes);
        size_x = Uint16(bytes);
        size_y = Uint16(bytes);
        game_length = Uint16(bytes);
        explosion_radius = Uint16(bytes);
        bomb_timer = Uint16(bytes);
    }

    void execute(GameState &game_state) override {
        game_state.server_name = server_name;
        game_state.players_count = players_count;
        game_state.size_x = size_x;
        game_state.size_y = size_y;
        game_state.game_length = game_length;
        game_state.explosion_radius = explosion_radius;
        game_state.bomb_timer = bomb_timer;
    }
};

class AcceptedPlayerMessage: public ServerMessage {
    PlayerId id;
    Player player;

public:
    explicit AcceptedPlayerMessage(Bytes &bytes) : id(PlayerId(bytes)), player(Player(bytes)) {}

    void execute(GameState &game_state) override {
        game_state.players.map[std::make_shared<PlayerId>(id)] = std::make_shared<Player>(player);
    }
};

class GameStartedMessage: public ServerMessage {
    PlayersMap players;

public:
    explicit GameStartedMessage(Bytes &bytes) : players(PlayersMap(bytes)) {}

    void execute(GameState &game_state) override {
        game_state.players = players;
    }
};

class TurnMessage: public ServerMessage {
    Uint16 turn;
    List<Event> events;
};

//class GuiMessage: public MessageReceived {};

std::shared_ptr<ServerMessage> get_server_message(Bytes bytes);

//std::shared_ptr<GuiMessage> get_gui_message(Bytes bytes);

#endif // ROBOTS_MESSAGES_H
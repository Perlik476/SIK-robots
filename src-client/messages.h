#ifndef ROBOTS_MESSAGES_H
#define ROBOTS_MESSAGES_H

#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "definitions.h"

class ClientMessage: public Serializable {
private:
    virtual char get_identifier() const = 0;
public:
    void send(boost::asio::ip::tcp::socket &socket) {
        Bytes message = serialize();
        socket.send(boost::asio::buffer((std::vector<char>) message));
    }

    Bytes serialize() const override {
        return {get_identifier()};
    }
};

class JoinMessage : public ClientMessage {
private:
    char get_identifier() const override { return 0; }
    String name;

public:
    JoinMessage() = default;

    explicit JoinMessage(const std::string &name): name(String(name)) {}

    explicit JoinMessage(const String &name): name(name) {}

    Bytes serialize() const override {
        return Bytes(get_identifier()) + name.serialize();
    }
};

class PlaceBombMessage : public ClientMessage {
private:
    char get_identifier() const override { return 1; }

public:
    PlaceBombMessage() = default;
};

class PlaceBlockMessage : public ClientMessage {
private:
    char get_identifier() const override { return 2; }

public:
    PlaceBlockMessage() = default;
};

class MoveMessage : public ClientMessage {
private:
    char get_identifier() const override { return 3; }
    Direction direction = Undefined;

public:
    MoveMessage() = default;

    explicit MoveMessage(const Direction direction): direction(direction) {}

    Bytes serialize() const override {
        return Bytes(get_identifier()) + Bytes(direction);
    }
};


class DrawMessage: public Serializable {
private:
    virtual char get_identifier() const = 0;
public:
    void send(boost::asio::ip::udp::socket &socket,
              boost::asio::ip::udp::endpoint &endpoint) {
        Bytes message = serialize();
        socket.send_to(boost::asio::buffer((std::vector<char>) message), endpoint);
    }
};

class LobbyMessage : public DrawMessage {
private:
    char get_identifier() const override { return 0; }
    GameState game_state;

public:
    LobbyMessage() = default;

    explicit LobbyMessage(GameState &game_state) : game_state(game_state) {}

    Bytes serialize() const override {
        return Uint8(static_cast<uint8_t>(get_identifier())).serialize()
            + game_state.server_name.serialize()
            + game_state.players_count.serialize()
            + game_state.size_x.serialize()
            + game_state.size_y.serialize()
            + game_state.game_length.serialize()
            + game_state.explosion_radius.serialize()
            + game_state.bomb_timer.serialize()
            + game_state.players.serialize();
    }
};

class GameMessage : public DrawMessage {
private:
    char get_identifier() const override { return 1; }
    GameState game_state;

public:
    GameMessage() = default;

    explicit GameMessage(GameState &game_state) : game_state(game_state) {}

    Bytes serialize() const override {
        return Uint8(static_cast<uint8_t>(get_identifier())).serialize()
               + game_state.server_name.serialize()
               + game_state.size_x.serialize()
               + game_state.size_y.serialize()
               + game_state.game_length.serialize()
               + game_state.turn.serialize()
               + game_state.players.serialize()
               + game_state.player_positions.serialize()
               + game_state.blocks.serialize()
               + game_state.bombs.serialize()
               + game_state.explosions.serialize()
               + game_state.scores.serialize();
    }
};


//class MessageReceived {
//public:
//    MessageReceived() = default;
//
//    virtual void execute(GameState &game_state) = 0;
//};

enum ServerMessageType: char {
    Hello,
    AcceptedPlayer,
    GameStarted,
    Turn,
    GameEnded
};

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
        game_state.players.map[id] = std::make_shared<Player>(player);
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

public:
    explicit TurnMessage(Bytes &bytes) : turn(Uint16(bytes)), events(List<Event>(bytes)) {}

    void execute(GameState &game_state) override {
        // TODO
        std::cout << "TurnMessage:\nturn: " << turn.value << "\n";
        std::cout << "list length: " << events.list.size() << "\n";
        for (auto &event : events.list) {
            event->execute(game_state);
        }
    }
};

class GameEndedMessage: public ServerMessage {
    PlayerScoresMap scores;

public:
    explicit GameEndedMessage(Bytes &bytes) : scores(PlayerScoresMap(bytes)) {}

    void execute(GameState &game_state) override {
        // TODO
    }
};


enum GuiMessageType: char {
    PlaceBomb,
    PlaceBlock,
    Move
};

class GuiMessage: public Executable {};

class PlaceBombGuiMessage: public GuiMessage {
public:
    PlaceBombGuiMessage() = default;

    void execute(GameState &game_state) override {
        game_state.place_bomb(*game_state.player_positions.map[game_state.my_id]);
    }
};

class PlaceBlockGuiMessage: public GuiMessage {
public:
    PlaceBlockGuiMessage() = default;

    void execute(GameState &game_state) override {
        game_state.place_block(*game_state.player_positions.map[game_state.my_id]);
    }
};

class MoveGuiMessage: public GuiMessage {
    Direction direction;
public:
    explicit MoveGuiMessage(Bytes &bytes) : direction((Direction) bytes.get_next_byte()) {} // TODO

    void execute(GameState &game_state) override {
        game_state.try_move(direction, game_state.player_positions.map[game_state.my_id]);
    }
};

//class GuiMessage: public MessageReceived {};

std::shared_ptr<ServerMessage> get_server_message(Bytes &bytes);

std::shared_ptr<GuiMessage> get_gui_message(Bytes &bytes);

//std::shared_ptr<GuiMessage> get_gui_message(Bytes &&bytes);

#endif // ROBOTS_MESSAGES_H
#ifndef ROBOTS_SERVER_MESSAGES_H
#define ROBOTS_SERVER_MESSAGES_H

#include "../definitions.h"
#include "draw_messages.h"

enum ServerMessageType: char {
    Hello,
    AcceptedPlayer,
    GameStarted,
    Turn,
    GameEnded,
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

    void execute(GameState &game_state, SocketsInfo &sockets_info) override {
        game_state.server_name = server_name;
        game_state.players_count = players_count;
        game_state.size_x = size_x;
        game_state.size_y = size_y;
        game_state.game_length = game_length;
        game_state.explosion_radius = explosion_radius;
        game_state.bomb_timer = bomb_timer;
        std::cout << "Hello: " << server_name.get_string() << ": " << (int) players_count.get_value() << ", " << size_x.get_value() << "x" <<
                  size_y.get_value() << ", " << game_length.get_value() << ", " << explosion_radius.get_value() << ", " << bomb_timer.get_value() << std::endl;
        LobbyMessage(game_state).send(sockets_info.get_gui_socket(), sockets_info.get_gui_endpoint());
    }
};

class AcceptedPlayerMessage: public ServerMessage {
    PlayerId id;
    Player player;

public:
    explicit AcceptedPlayerMessage(Bytes &bytes) : id(PlayerId(bytes)), player(Player(bytes)) {}

    void execute(GameState &game_state, SocketsInfo &sockets_info) override {
        game_state.players.get_map()[id] = Player(player.get_name(), player.get_address());
        std::cout << "AcceptedPlayer: " << id.get_value() << ": " << player.get_name().get_string() << ", " << player.get_address().get_string() << std::endl;
        LobbyMessage(game_state).send(sockets_info.get_gui_socket(), sockets_info.get_gui_endpoint());
    }
};

class GameStartedMessage: public ServerMessage {
    players_t players;

public:
    explicit GameStartedMessage(Bytes &bytes) : players(players_t(bytes)) {}

    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override {
        game_state.players = players;
        std::cout << "GameStarted: \n";
        game_state.scores = players_scores_t();
        for (auto &[key, value] : game_state.players.get_map()) {
            std::cout << (int) key.get_value() << ": name: " << value.get_name().get_string() << ", addr: " << value.get_address().get_string() << "\n";
            game_state.scores.get_map()[key] = Score(0);
        }
    }
};

class TurnMessage: public ServerMessage {
    Uint16 turn;
    List<Event> events;

public:
    explicit TurnMessage(Bytes &bytes) : turn(Uint16(bytes)), events(List<Event>(bytes)) {}

    void execute(GameState &game_state, SocketsInfo &sockets_info) override {
        // TODO
        std::cout << "TurnMessage:\nturn: " << turn.get_value() << "\n";
        std::cout << "list length: " << events.get_list().size() << "\n";

        for (auto &bomb : game_state.bombs.get_list()) {
            bomb->timer -= 1;
        }

        game_state.prepare_for_turn();

        game_state.turn = turn.get_value();
        for (auto &event : events.get_list()) {
            event.execute(game_state, sockets_info);
        }

        game_state.after_turn();

        // TODO mapa z bombami

        GameMessage(game_state).send(sockets_info.get_gui_socket(), sockets_info.get_gui_endpoint());
    }
};

class GameEndedMessage: public ServerMessage {
    players_scores_t scores;

public:
    explicit GameEndedMessage(Bytes &bytes) : scores(players_scores_t(bytes)) {}

    void execute(GameState &game_state, SocketsInfo &sockets_info) override {
        game_state.scores = scores;
        game_state.is_joined = false;
        game_state.reset();
        LobbyMessage(game_state).send(sockets_info.get_gui_socket(), sockets_info.get_gui_endpoint());
    }
};

#endif //ROBOTS_SERVER_MESSAGES_H

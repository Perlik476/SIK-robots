#ifndef ROBOTS_DRAW_MESSAGES_H
#define ROBOTS_DRAW_MESSAGES_H

#include "../includes.h"

class DrawMessage: public Serializable {
private:
    virtual char get_identifier() const = 0;
public:
    void send(const gui_socket_t &socket, const udp::endpoint &endpoint) const {
        Bytes message = serialize();
        socket->send_to(boost::asio::buffer(message.get_vector()), endpoint);
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
               + game_state.get_server_name().serialize()
               + game_state.get_players_count().serialize()
               + game_state.get_size_x().serialize()
               + game_state.get_size_y().serialize()
               + game_state.get_game_length().serialize()
               + game_state.get_explosion_radius().serialize()
               + game_state.get_bomb_timer().serialize()
               + game_state.get_players().serialize();
    }
};

class GameMessage: public DrawMessage {
private:
    char get_identifier() const override { return 1; }
    GameState game_state;

public:
    GameMessage() = delete;

    explicit GameMessage(GameState &game_state) : game_state(game_state) {}

    Bytes serialize() const override {
        return Uint8(static_cast<uint8_t>(get_identifier())).serialize()
               + game_state.get_server_name().serialize()
               + game_state.get_size_x().serialize()
               + game_state.get_size_y().serialize()
               + game_state.get_game_length().serialize()
               + game_state.get_turn().serialize()
               + game_state.get_players().serialize()
               + game_state.get_player_positions().serialize()
               + game_state.get_blocks().serialize()
               + game_state.get_bombs().serialize()
               + game_state.get_explosions().serialize()
               + game_state.get_scores().serialize();
    }
};

#endif //ROBOTS_DRAW_MESSAGES_H

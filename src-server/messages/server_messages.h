#ifndef ROBOTS_SERVER_MESSAGES_H
#define ROBOTS_SERVER_MESSAGES_H

#include "../includes.h"

class ServerMessage : public Serializable {
private:
    virtual char get_identifier() const = 0;
public:
    void send(const socket_t &socket) const {
        Bytes message = serialize();
        socket->send(boost::asio::buffer(message.get_vector()));
    }

    Bytes serialize() const override {
        return {get_identifier()};
    }
};

class HelloMessage : public ServerMessage {
    char get_identifier() const override {
        return 0;
    }

    String server_name;
    players_count_t players_count;
    coordinate_t size_x;
    coordinate_t size_y;
    game_length_t game_length;
    explosion_radius_t explosion_radius;
    bomb_timer_t bomb_timer;
public:
    HelloMessage(std::shared_ptr<GameState> &game_state)
        : server_name(game_state->get_server_name()),
        players_count(game_state->get_players_count()),
        size_x(game_state->get_size_x()),
        size_y(game_state->get_size_y()),
        game_length(game_state->get_game_length()),
        explosion_radius(game_state->get_explosion_radius()),
        bomb_timer(game_state->get_bomb_timer()) {}

    Bytes serialize() const override {
        std::cout << "Hello" << std::endl;
        return Bytes(get_identifier()) + server_name.serialize() + players_count.serialize() + size_x.serialize()
            + size_y.serialize() + game_length.serialize() + explosion_radius.serialize() + bomb_timer.serialize();
    }
};


class AcceptedPlayerMessage : public ServerMessage {
    char get_identifier() const override {
        return 1;
    }

    player_id_t id;
    Player player;
public:
    AcceptedPlayerMessage(player_id_t &id, Player &player) : id(id), player(player) {}

    Bytes serialize() const override {
        std::cout << "AcceptedPlayer:" << player.get_name().get_string() << " " << player.get_address().get_string() << std::endl;
        return Bytes(get_identifier()) + id.serialize() + player.serialize();
    }
};

#endif //ROBOTS_SERVER_MESSAGES_H

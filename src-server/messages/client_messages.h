#ifndef ROBOTS_CLIENT_MESSAGES_H
#define ROBOTS_CLIENT_MESSAGES_H

#include "../includes.h"

enum ClientMessageType: char {
    Join,
    PlaceBomb,
    PlaceBlock,
    Move,
};

class ClientMessage: public Executable {
protected:
    player_id_t player_id;
};

class PlaceBombMessage : public ClientMessage {
public:
    explicit PlaceBombMessage(player_id_t &player_id) {
        this->player_id = player_id;
    }

    void execute(GameState &game_state) override {

    }
};
//
//class JoinMessage : public ClientMessage {
//private:
//    char get_identifier() const override { return 0; }
//    String name;
//
//public:
//    JoinMessage() = default;
//
//    explicit JoinMessage(const std::string &name): name(String(name)) {}
//
//    explicit JoinMessage(const String &name): name(name) {}
//
//    Bytes serialize() const override {
//        return Bytes(get_identifier()) + name.serialize();
//    }
//};

//
//class PlaceBlockMessage : public ClientMessage {
//private:
//    char get_identifier() const override { return 2; }
//
//public:
//    PlaceBlockMessage() = default;
//};
//
//class MoveMessage : public ClientMessage {
//private:
//    char get_identifier() const override { return 3; }
//    Direction direction = Undefined;
//
//public:
//    MoveMessage() = default;
//
//    explicit MoveMessage(const Direction direction): direction(direction) {}
//
//    Bytes serialize() const override {
//        return Bytes(get_identifier()) + Bytes(direction);
//    }
//};

#endif //ROBOTS_CLIENT_MESSAGES_H

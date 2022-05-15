#ifndef ROBOTS_MESSAGES_H
#define ROBOTS_MESSAGES_H

#include "definitions.h"

class Message: public Serializable {
public:
    virtual void send() = 0;
};

class ClientMessage: public Message {
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


class DrawMessage: public Message {
private:
    virtual char get_identifier() = 0;
public:
    void send() override {
        Bytes message = serialize();
    }
};

class LobbyMessage : public DrawMessage {
private:
    char get_identifier() override { return 0; }
    String server_name;
    Uint<uint8_t> players_count;
    Uint<uint16_t> size_x;
    Uint<uint16_t> size_t;
    Uint<uint16_t> game_length;
    Uint<uint16_t> explosion_radius;
    Uint<uint16_t> bomb_timer;
    Map<player_id_t, Player> players;

public:
    LobbyMessage() = default;

//    explicit LobbyMessage(const Direction direction): direction(direction) {}

//    Bytes serialize() override {
//        return Bytes(get_identifier()) + Bytes(direction);
//    }
};



#endif // ROBOTS_MESSAGES_H
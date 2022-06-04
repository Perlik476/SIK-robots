#ifndef ROBOTS_CLIENT_MESSAGES_H
#define ROBOTS_CLIENT_MESSAGES_H

#include "../includes.h"

class ClientMessage: public Serializable {
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

#endif //ROBOTS_CLIENT_MESSAGES_H

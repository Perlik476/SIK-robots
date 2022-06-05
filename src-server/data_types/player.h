#ifndef ROBOTS_PLAYER_H
#define ROBOTS_PLAYER_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"
#include "string.h"

using player_id_t = Uint8;

class Player: public Serializable {
private:
    String name;
    String address;
public:
    Player() = default;

    explicit Player(const String &name, const String &address): name(name), address(address) {}

    explicit Player(Bytes &bytes) {
        name = String(bytes);
        address = String(bytes);
    }

    String get_name() const { return name; }

    String get_address() const { return address; }

    Bytes serialize() const override {
        return name.serialize() + address.serialize();
    }

    int operator== (Player &other) {
        return address.get_string() == other.address.get_string(); // TODO
    }
};

#endif //ROBOTS_PLAYER_H

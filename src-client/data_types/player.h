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

    String get_name() { return name; }

    String get_address() { return address; }

    Bytes serialize() const override {
        return name.serialize() + address.serialize();
    }
};

#endif //ROBOTS_PLAYER_H

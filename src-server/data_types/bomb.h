#ifndef ROBOTS_BOMB_H
#define ROBOTS_BOMB_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"
#include "position.h"

using bomb_id_t = Uint32;

class Bomb: public Serializable {
    Position position;
    Uint16 timer;
public:

    Bomb() = default;

    explicit Bomb(Position &position, Uint16 &timer): position(position), timer(timer) {}

    explicit Bomb(Bytes &bytes) {
        position = Position(bytes);
        timer = Uint16(bytes);
    }

    Bytes serialize() const override {
        return position.serialize() + timer.serialize();
    }

    void decrease_timer() { timer -= 1; }

    bool does_explode() { return !timer.get_value(); }

    auto get_position() { return position; }
};

#endif //ROBOTS_BOMB_H

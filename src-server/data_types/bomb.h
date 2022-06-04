#ifndef ROBOTS_BOMB_H
#define ROBOTS_BOMB_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"
#include "position.h"

using BombId = Uint32;

class Bomb: public Serializable {
public:
    Position position;
    Uint16 timer;

    Bomb() = default;

    explicit Bomb(Position &position, Uint16 &timer): position(position), timer(timer) {}

    explicit Bomb(Bytes &bytes) {
        position = Position(bytes);
        timer = Uint16(bytes);
    }

    Bytes serialize() const override {
        return position.serialize() + timer.serialize();
    }

    bool operator==(const Bomb &other) const {
        return position == other.position && timer == other.timer;
    }
};

#endif //ROBOTS_BOMB_H

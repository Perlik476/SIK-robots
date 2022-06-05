#ifndef ROBOTS_EVENTS_H
#define ROBOTS_EVENTS_H

#include "definitions.h"
#include "bytes.h"
#include "uint.h"
#include "position.h"
#include "bomb.h"
#include "player.h"
#include "list.h"

enum EventType: char {
    BombPlaced,
    BombExploded,
    PlayerMoved,
    BlockPlaced,
};

class Event: public Serializable {};

class BombPlacedEvent: public Event {
    bomb_id_t id;
    Position position;

public:
    explicit BombPlacedEvent(bomb_id_t &id, Position &position) : id(id), position(position) {}

    Bytes serialize() const override {
        return Bytes(EventType::BombPlaced) + id.serialize() + position.serialize();
    }
};

class BombExplodedEvent: public Event {
    bomb_id_t id;
    List<player_id_t> robots_destroyed;
    List<Position> blocks_destroyed;
public:
    explicit BombExplodedEvent(const bomb_id_t &id, List<player_id_t> &robots_destroyed, List<Position> &blocks_destroyed)
        : id(id), robots_destroyed(robots_destroyed), blocks_destroyed(blocks_destroyed) {}

    Bytes serialize() const override {
        return Bytes(EventType::BombExploded) + id.serialize() + robots_destroyed.serialize()
            + blocks_destroyed.serialize();
    }
};

class PlayerMovedEvent: public Event {
    player_id_t id;
    Position position;

public:
    explicit PlayerMovedEvent(player_id_t &id, Position &position) : id(id), position(position) {}

    Bytes serialize() const override {
        return Bytes(EventType::PlayerMoved) + id.serialize() + position.serialize();
    }
};

class BlockPlacedEvent: public Event {
    Position position;

public:
    explicit BlockPlacedEvent(Position &position) : position(position) {}

    Bytes serialize() const override {
        return Bytes(EventType::BlockPlaced) + position.serialize();
    }
};

#endif //ROBOTS_EVENTS_H

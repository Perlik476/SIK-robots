#ifndef ROBOTS_ACTION_H
#define ROBOTS_ACTION_H

#include "definitions.h"
#include "player.h"

class GameState;
class Event;

class Action {
public:
    virtual std::shared_ptr<Event> execute(GameState *game_state, uint8_t &player_id) = 0;

    virtual ~Action() = default;
};

class MoveAction : public Action {
    Direction direction;
public:
    MoveAction(Direction direction) : direction(direction) {}

    std::shared_ptr<Event> execute(GameState *game_state, uint8_t &player_id) override;
};

class PlaceBombAction {

};

class PlaceBlockAction {

};

#endif //ROBOTS_ACTION_H

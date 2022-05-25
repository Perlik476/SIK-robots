#ifndef ROBOTS_POSITION_H
#define ROBOTS_POSITION_H

#include "usings.h"
#include "uint.h"

using Coordinate = Uint16;

class Position: public Serializable {
    Coordinate x, y;
public:
    auto &get_x() { return x; }

    auto &get_y() { return y; }

    auto &get_x() const { return x; }

    auto &get_y() const { return y; }

    Position() = default;

    Position(uint16_t x, uint16_t y) : x(x), y(y) {}

    Position(Uint16 &x, Uint16 &y) : x(x), y(y) {}

//    Position(Position &position) : x(position.get_x()), y(position.get_y()) {}

    explicit Position(Bytes &bytes) {
        x = Coordinate(bytes);
        y = Coordinate(bytes);
    }

    Bytes serialize() const override {
        return x.serialize() + y.serialize();
    }

    Position up() const {
        return Position(x.get_value(), y.get_value() + 1);
    }

    Position right() const {
        return Position(x.get_value() + 1, y.get_value());
    }

    Position down() const {
        return Position(x.get_value(), y.get_value() - 1);
    }

    Position left() const {
        return Position(x.get_value() - 1, y.get_value());
    }

    Position next(Direction &direction) const {
        switch(direction) {
            case Direction::Up:
                return up();
            case Direction::Right:
                return right();
            case Direction::Down:
                return down();
            case Direction::Left:
                return left();
            default:
                throw std::invalid_argument("Argument is not a proper direction.");
        }
    }

    bool is_next_proper(Direction &direction, Uint16 size_x, Uint16 size_y) {
        switch(direction) {
            case Direction::Up:
                return y.get_value() < size_y.get_value() - 1;
            case Direction::Right:
                return x.get_value() < size_x.get_value() - 1;
            case Direction::Down:
                return y.get_value() > 0;
            case Direction::Left:
                return x.get_value() > 0;
            default:
                return false;
        }
    }

    bool operator<(Position const &other) const {
        return x < other.get_x() || (x == other.get_x() && y < other.get_y());
    }

    bool operator==(Position const &other) const {
        return x == other.get_x() && y == other.get_y();
    }
};

#endif //ROBOTS_POSITION_H

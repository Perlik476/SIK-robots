#ifndef ROBOTS_ARGUMENTS_H
#define ROBOTS_ARGUMENTS_H

#include "definitions.h"

class ArgumentsParsingFailed : public std::exception {
    std::string message = "Arguments parsing failed.";
public:
    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
        return message.c_str();
    }

    ArgumentsParsingFailed() = default;

    ArgumentsParsingFailed(const std::string &message) : message(message) {}
};

class Arguments {
    std::string server_name;
    uint8_t players_count;
    uint16_t port, bomb_timer, explosion_radius, initial_blocks, game_length, size_x, size_y;
    uint32_t seed;
    uint64_t turn_duration;

    static bool is_proper_string(std::string &s) {
        return s.length() < 256;
    }

    static bool is_proper_uint8(uint16_t x) {
        return x <= UINT8_MAX;
    }

    void parse_arguments(int argc, char *argv[]);

public:
    void check_correctness(uint16_t players_count_temp) {
        if (!is_proper_string(server_name)) {
            throw ArgumentsParsingFailed("Server name is too long.");
        }
        if (!is_proper_uint8(players_count_temp)) {
            throw ArgumentsParsingFailed("players-count is too big.");
        }
    }

public:
    friend class GameState;

    Arguments(int argc, char *argv[]) {
        parse_arguments(argc, argv);
    }

    uint16_t get_port() { return port; }
};

#endif //ROBOTS_ARGUMENTS_H

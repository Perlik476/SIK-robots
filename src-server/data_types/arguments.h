#ifndef ROBOTS_ARGUMENTS_H
#define ROBOTS_ARGUMENTS_H

#include "definitions.h"

class ArgumentsParsingFailed : public std::exception {
public:
    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
        return "Arguments parsing failed.";
    }
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

    void parse_arguments(int argc, char *argv[]);

public:
    static std::pair<std::string, std::string> process_address(std::string &address) {
        int length = (int) address.length();

        std::string address_pure, port_str;

        for (int i = length - 1; i >= 0; i--) {
            char c = address[(size_t) i];
            if (c == ':') {
                for (size_t j = 0; j < (size_t) i; j++) {
                    address_pure += address[j];
                }
                break;
            }
            if (c < '0' || c > '9') {
                throw ArgumentsParsingFailed();
            }
            port_str += c;
        }
        std::reverse(port_str.begin(), port_str.end());

        return { address_pure, port_str };
    }

    bool check_correctness() {
        return is_proper_string(server_name);
    }

public:
    Arguments(int argc, char *argv[]) {
        parse_arguments(argc, argv);
    }
//    Arguments(std::string server_name, uint8_t players_count, uint16_t port, uint16_t bomb_timer,
//              uint16_t explosion_radius, uint16_t initial_blocks, uint16_t game_length,
//              uint16_t size_x, uint16_t size_y, uint32_t seed, uint64_t turn_duration)
//              : server_name(server_name), players_count(players_count), port(port), bomb_timer(bomb_timer),
//              explosion_radius(explosion_radius), initial_blocks(initial_blocks), game_length(game_length),
//              size_x(size_x), size_y(size_y), seed(seed), turn_duration(turn_duration) {}
};

#endif //ROBOTS_ARGUMENTS_H

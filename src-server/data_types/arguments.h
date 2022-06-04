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
    // TODO
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
    friend class GameState;

    Arguments(int argc, char *argv[]) {
        parse_arguments(argc, argv);
    }

    uint16_t get_port() { return port; }
};

#endif //ROBOTS_ARGUMENTS_H

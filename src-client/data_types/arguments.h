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
    static bool is_proper_string(std::string &s) {
        return s.length() < 256;
    }

public:
    std::string player_name;
    uint16_t port;
    std::string gui_address;
    std::string server_address;

    std::string gui_address_pure;
    std::string gui_port;
    std::string server_address_pure;
    std::string server_port;

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
        return is_proper_string(player_name);
    }

public:
    Arguments(std::string player_name, uint16_t port, std::string gui_address, std::string server_address)
            : player_name(std::move(player_name)), port(port), gui_address(std::move(gui_address)),
              server_address(std::move(server_address)) {

        auto pair = process_address(this->gui_address);
        gui_address_pure = pair.first;
        gui_port = pair.second;

        pair = process_address(this->server_address);
        server_address_pure = pair.first;
        server_port = pair.second;
    }
};

#endif //ROBOTS_ARGUMENTS_H

#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <utility>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <ranges>

#include "definitions.h"
#include "messages.h"

namespace po = boost::program_options;

class Arguments {
public:
    std::string player_name;
    uint16_t port;
    std::string gui_address;
    std::string server_address;

public:
    Arguments(std::string player_name, uint16_t port, std::string gui_address, std::string server_address)
        : player_name(std::move(player_name)), port(port), gui_address(std::move(gui_address)),
        server_address(std::move(server_address)) {}
};

std::shared_ptr<Arguments> parse_arguments(int argc, char *argv[]) {
    std::string gui_address, player_name, server_address;
    uint16_t port;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("gui-address,d", po::value<std::string>(&gui_address))
            ("help,h", "Produce help message")
            ("player-name,n", po::value<std::string>(&player_name))
            ("port,p", po::value<uint16_t>(&port), "Port for GUI communication")
            ("server-address,s", po::value<std::string>(&server_address));

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    return make_shared<Arguments>(player_name, port, gui_address, server_address);
}

int main(int argc, char *argv[]) {

    auto arguments = parse_arguments(argc, argv);
    std::cout << "gui_address: " << arguments->gui_address << "\n"
        << "server_address: " << arguments->server_address << "\n"
        << "port: " << arguments->port << "\n"
        << "player_name: " << arguments->player_name << "\n";

    List<String> list;
    auto string1 = std::make_shared<String>();
    string1->string = "lol";
    auto string2 = std::make_shared<String>();
    string2->string = "XDD";
    list.list.push_back(string1);
    list.list.push_back(string2);

    auto bytes = list.serialize();
    uint32_t len;
    memcpy(&len, &bytes.front(), 4);
    len = ntohl(len);
    std::cout << len << " " << bytes.size() << "\n";

    std::cout << "Move:\n";
    auto bytes2 = MoveMessage(Direction::Down).serialize();
    for (auto &byte : bytes2) {
        std::cout << (int) byte << "\n";
    }

    std::cout << "Join:\n";

    auto bytes3 = JoinMessage("XD").serialize();
    for (auto &byte : bytes3) {
        std::cout << (int) byte << ": " << byte << "\n";
    }

    return 0;
}
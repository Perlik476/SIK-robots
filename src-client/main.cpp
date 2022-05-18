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

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class Arguments {
public:
    std::string player_name;
    uint16_t port;
    std::string gui_address;
    std::string server_address;

    std::string gui_address_pure;
    std::string gui_port;
    std::string server_address_pure;
    uint16_t server_port;

    std::pair<std::string, std::string> process_address(std::string &address) {
        int length = (int) address.length();
        std::cout << length << "\n";

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
                exit(1);
            }
            port_str += c;
        }
        std::reverse(port_str.begin(), port_str.end());

        return { address_pure, port_str };
    }

public:
    Arguments(std::string player_name, uint16_t port, std::string gui_address, std::string server_address)
        : player_name(std::move(player_name)), port(port), gui_address(std::move(gui_address)),
        server_address(std::move(server_address)) {

        auto pair = process_address(this->gui_address);
        gui_address_pure = pair.first;
        gui_port = pair.second;
    }
};

std::shared_ptr<Arguments> parse_arguments(int argc, char *argv[]) {
    std::string gui_address, player_name, server_address;
    uint16_t port;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "Produce help message")
            ("gui-address,d", po::value<std::string>(&gui_address)->required())
            ("player-name,n", po::value<std::string>(&player_name)->required())
            ("port,p", po::value<uint16_t>(&port)->required(), "Port for GUI communication")
            ("server-address,s", po::value<std::string>(&server_address)->required());

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    po::notify(vm);

    return make_shared<Arguments>(player_name, port, gui_address, server_address);
}

int main(int argc, char *argv[]) {

    auto arguments = parse_arguments(argc, argv);
    std::cout << "gui_address: " << arguments->gui_address << "\n"
        << "server_address: " << arguments->server_address << "\n"
        << "port: " << arguments->port << "\n"
        << "player_name: " << arguments->player_name << "\n";


    boost::asio::io_context io_context;

    udp::resolver resolver(io_context);
    udp::endpoint receiver_endpoint = *resolver.resolve(udp::v4(), "localhost", "2137").begin();
    udp::socket socket_gui_send(io_context);
    socket_gui_send.open(udp::v4());

    GameState game_state;
    game_state.server_name = String("XD");
    game_state.players_count = Uint8(2);
    game_state.size_x = Uint16(6);
    game_state.size_y = Uint16(6);
    game_state.game_length = Uint16(10);
    game_state.explosion_radius = Uint16(3);
    game_state.bomb_timer = Uint16(3);
    game_state.players = PlayersMap();
    game_state.turn = 3;
    Uint8 id = Uint8(0);
    game_state.players.map[id] =
            std::make_shared<Player>(String(arguments->player_name), String("127.0.0.1:2138"));

    game_state.player_positions.map[id] = std::make_shared<Position>(1, 1);

    game_state.blocks.list.push_back(std::make_shared<Position>(2, 3));
    game_state.blocks.list.push_back(std::make_shared<Position>(3, 4));
    game_state.scores.map[id] = std::make_shared<Score>(0);

    game_state.my_id = 0;

//    game_state.bombs.list.push_back()

    auto lobby_message = LobbyMessage(game_state);
//    Bytes bytes = lobby_message.serialize();
//    for (size_t i = 0; i < bytes.size(); i++) {
//        std::cout << (int) bytes[i] << " ";
//    }
//    std::cout << "\n";
    lobby_message.send(socket_gui_send, receiver_endpoint);

    sleep(2);

    auto game_message = GameMessage(game_state);
//    Bytes bytes = game_message.serialize();
//    for (size_t i = 0; i < bytes.size(); i++) {
//        std::cout << (int) bytes[i] << " ";
//    }
//    std::cout << "\n";

    game_message.send(socket_gui_send, receiver_endpoint);

    udp::socket socket_gui_receive(io_context, udp::endpoint(udp::v4(), arguments->port));

    for (;;) {
        boost::array<char, 256> recv_buf;
        udp::endpoint remote_endpoint;
        size_t size = socket_gui_receive.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

        for (size_t i = 0; i < size; i++) {
            std::cout << (int) recv_buf[i] << " ";
        }
        std::cout << "\n";

        Bytes bytes = Bytes(get_c_array(recv_buf), size);
        auto message = get_gui_message(bytes);
        message->execute(game_state);
        GameMessage(game_state).send(socket_gui_send, receiver_endpoint);
    }

//    tcp::resolver resolver(io_context);
//    std::cout << arguments->gui_address_pure << ":" << arguments->gui_port << ":" << arguments->gui_port <<     "\n";
//    tcp::resolver::results_type endpoints = resolver.resolve(arguments->gui_address_pure, arguments->gui_port);
//    tcp::socket_gui_send socket_gui_send(io_context);
//    boost::asio::connect(socket_gui_send, endpoints);

//    tcp::resolver resolver(io_context);
//    tcp::resolver::results_type endpoints = resolver.resolve("localhost", "2138");
//    tcp::socket_gui_send socket_gui_send(io_context);
//    boost::asio::connect(socket_gui_send, endpoints);
//
//    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), arguments->port));
//    tcp::socket_gui_send socket2(io_context);
//    acceptor.accept(socket2);

//    boost::asio::io_context io_context;
//    tcp::resolver resolver(io_context);
//    std::cout << arguments->gui_address_pure << ":" << arguments->gui_port << ":" << arguments->gui_port <<     "\n";
//    tcp::resolver::results_type endpoints = resolver.resolve(arguments->gui_address_pure, arguments->gui_port);
//    tcp::socket_gui_send socket_gui_send(io_context);
//    boost::asio::connect(socket_gui_send, endpoints);

    std::cout << "done\n";

//    List<String> list;
//    auto string1 = std::make_shared<String>();
//    string1->string = "lol";
//    auto string2 = std::make_shared<String>();
//    string2->string = "XDDDD";
//    list.list.push_back(string1);
//    list.list.push_back(string2);
//
//    auto bytes = list.serialize();
//    uint32_t len;
//    memcpy(&len, &bytes.front(), 4);
//    len = ntohl(len);
//    std::cout << len << " " << bytes.size() << "\n";
//
//    std::cout << Uint32(bytes).value << "\n";
//
//    bytes.reset_pointer();
//    List<String> deserialized = List<String>(bytes);
//
//    std::cout << deserialized.list.size() << " " << (*deserialized.list[0]).string << " " << (*deserialized.list[1]).string << "\n";
//
//    GameState game_state;
//
//    std::vector<char> v = {3, 0, 44, 0, 0, 0, 3, 2, 3, 0, 2, 0, 4, 2, 4, 0, 3, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 7};
//    Bytes bytes2 = Bytes(v);
//
//    auto message = get_server_message(bytes2);
//    message->execute(game_state);

//    std::cout << "Move:\n";
//    auto bytes2 = MoveMessage(Direction::Down).serialize();
//    for (auto &byte : bytes2) {
//        std::cout << (int) byte << "\n";
//    }
//
//    std::cout << "Join:\n";
//
//    auto bytes3 = JoinMessage("XDDD").serialize();
//    for (auto &byte : bytes3) {
//        std::cout << (int) byte << ": " << byte << "\n";
//    }
//
//    for (size_t i = 0; i < bytes3.size(); i++) {
//        std::cout << (int) Uint8(bytes3).value << " ";
//    }

    return 0;
}
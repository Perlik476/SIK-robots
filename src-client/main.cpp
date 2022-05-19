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
    std::string server_port;

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

        pair = process_address(this->server_address);
        server_address_pure = pair.first;
        server_port = pair.second;
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

//void receive_from_server(GameState &game_state) {
//
//}
//


//void test(GameState &x, udp::socket &y){
//    std::cout << (int) x.my_id.value << std::endl;
//}
//
//int main(int argc, char *argv[]) {
//    GameState x;
//    x.my_id.value = 7;
//
//    boost::asio::io_context io_context;
//
//    udp::resolver resolver(io_context);
////    udp::endpoint receiver_endpoint = *resolver.resolve("localhost", "2137").begin();
////    udp::socket socket_gui_send(io_context);
////    socket_gui_send.open(receiver_endpoint.protocol());
//    udp::socket socket_gui_receive(io_context, udp::endpoint(udp::v6(), 2137));
//
//    std::thread t{test, std::ref(x), std::ref(socket_gui_receive)};
//    t.join();
//
//    return 0;
//}

void receive_from_gui(Arguments &arguments, GameState &game_state, udp::socket &socket_gui, socket_tcp &socket_server,
                      udp::endpoint &gui_endpoint) {
    for (;;) {
        boost::array<char, 256> recv_buf;
        udp::endpoint remote_endpoint;
        size_t size = socket_gui.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

        for (size_t i = 0; i < size; i++) {
            std::cout << (int) recv_buf[i] << " ";
        }
        std::cout << "\n";

        Bytes bytes = Bytes(get_c_array(recv_buf), size);
        while (!bytes.is_end()) {
            auto message = get_gui_message(bytes);
            message->execute(game_state, socket_gui, socket_server, gui_endpoint);
        }
    }
}

void receive_from_server(Arguments &arguments, GameState &game_state, udp::socket &socket_gui,
                         socket_tcp &socket_server, udp::endpoint &gui_endpoint) {
    JoinMessage(arguments.player_name).send(socket_server);

    std::cout << "sent to server." << std::endl;

//    boost::array<char, 128> buf;
//    boost::system::error_code error;

    for (;;) {
//        std::cout << "\n\n";
//        size_t size = socket_server->read_some(boost::asio::buffer(buf), error);
//        for (size_t i = 0; i < size; i++)
//            std::cout << (int) buf[i] << " ";
//        std::cout << std::endl;
//        std::cout.write(buf.data(), size);
//        std::cout << std::endl;
//        Bytes bytes = Bytes(get_c_array(buf), size);
        BytesReceiver bytes = BytesReceiver(socket_server);
        while (!bytes.is_end()) {
            auto message = get_server_message(bytes);
            message->execute(game_state, socket_gui, socket_server, gui_endpoint);
        }
//        MoveMessage(Direction::Right).send(socket_server);
//        PlaceBombMessage().send(socket_server);
    }
}

int main(int argc, char *argv[]) {

    auto arguments = parse_arguments(argc, argv);
    std::cout << "gui_address: " << arguments->gui_address << "\n"
        << "server_address: " << arguments->server_address << "\n"
        << "port: " << arguments->port << "\n"
        << "player_name: " << arguments->player_name << "\n";


    boost::asio::io_context io_context;

    auto game_state = GameState();
//    game_state.server_name = String("XD");
//    game_state.players_count = Uint8(2);
//    game_state.size_x = Uint16(5);
//    game_state.size_y = Uint16(5);
//    game_state.game_length = Uint16(10);
//    game_state.explosion_radius = Uint16(3);
//    game_state.bomb_timer = Uint16(3);
//    game_state.players = PlayersMap();
//    game_state.turn = 3;
//    Uint8 id = Uint8(0);
//    game_state.players.map[id] =
//            std::make_shared<Player>(String(arguments->player_name), String("127.0.0.1:2138"));
//
//    game_state.player_positions.map[id] = std::make_shared<Position>(1, 1);
//
//    game_state.blocks.list.push_back(std::make_shared<Position>(2, 3));
//    game_state.blocks.list.push_back(std::make_shared<Position>(3, 4));
//    game_state.scores.map[id] = std::make_shared<Score>(0);
//
//    game_state.my_id = 0;

    udp::resolver resolver(io_context);
    udp::endpoint receiver_endpoint = *resolver.resolve(arguments->gui_address_pure, arguments->gui_port).begin();
    udp::socket socket_gui_send(io_context);
    socket_gui_send.open(receiver_endpoint.protocol());

//    auto lobby_message = LobbyMessage(game_state);
//    lobby_message.send(socket_gui_send, receiver_endpoint);
//
//    sleep(2);
//
//    auto game_message = GameMessage(game_state);
//
//    game_message.send(socket_gui_send, receiver_endpoint);

    udp::socket socket_gui_receive(io_context, udp::endpoint(udp::v6(), arguments->port));

    tcp::resolver resolver_tcp(io_context);
    tcp::resolver::results_type endpoints = resolver_tcp.resolve(arguments->server_address_pure, arguments->server_port);
    auto socket_server = std::make_shared<tcp::socket>(io_context);
    boost::asio::connect(*socket_server, endpoints);
    tcp::no_delay option(true);
    socket_server->set_option(option);

    std::thread receive_from_gui_thread{receive_from_gui, std::ref(*arguments), std::ref(game_state), std::ref(socket_gui_receive),
                                        std::ref(socket_server), std::ref(receiver_endpoint)};

    std::thread receive_from_server_thread{receive_from_server, std::ref(*arguments), std::ref(game_state),
                                           std::ref(socket_gui_receive), std::ref(socket_server), std::ref(receiver_endpoint)};

    receive_from_gui_thread.join();
    receive_from_server_thread.join();

    return 0;
}
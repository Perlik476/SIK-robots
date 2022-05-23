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


void receive_from_gui(Arguments &arguments, GameState &game_state, SocketsInfo &sockets_info) {
    for (;;) {
        boost::array<char, 3> recv_buf{}; // TODO
        udp::endpoint remote_endpoint;
        size_t size = sockets_info.gui_socket->receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

        std::cout << "packet (size = " << size << "): ";
        for (size_t i = 0; i < size; i++) {
            std::cout << (int) recv_buf[i] << " ";
        }
        std::cout << "\n";

        if (size == 0) {
            continue;
        }

        Bytes bytes = Bytes(get_c_array(recv_buf), size);
        auto message = get_gui_message(bytes);
        if (message != nullptr && bytes.is_end()) {
            if (!game_state.is_joined) {
                JoinMessage(arguments.player_name).send(sockets_info.server_socket);
                std::cout << "Join sent." << std::endl;
                game_state.is_joined = true;
            }
            else {
                std::cout << "message sent." << std::endl;
                message->execute(game_state, sockets_info);
            }
        }
    }
}

void receive_from_server(Arguments &arguments, GameState &game_state, SocketsInfo sockets_info) {
    for (;;) {
        BytesReceiver bytes = BytesReceiver(sockets_info.server_socket);
        while (!bytes.is_end()) {
            auto message = get_server_message(bytes);
            if (message == nullptr) {
                std::cout << "wrong message received from server.\n";
                sockets_info.server_socket->close();
                game_state.is_joined = false; // TODO
                return;
            }
            std::cout << "executing server message.\n";
            message->execute(game_state, sockets_info);
        }
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

    udp::resolver resolver(io_context);
    udp::endpoint gui_endpoint = *resolver.resolve(arguments->gui_address_pure, arguments->gui_port).begin();
    auto socket_gui_send = std::make_shared<udp::socket>(io_context);
    socket_gui_send->open(gui_endpoint.protocol());

    auto socket_gui_receive = std::make_shared<udp::socket>(io_context, udp::endpoint(udp::v6(), arguments->port));

    tcp::resolver resolver_tcp(io_context);
    tcp::endpoint endpoints = *resolver_tcp.resolve(arguments->server_address_pure, arguments->server_port);
    auto socket_server = std::make_shared<tcp::socket>(io_context);
    boost::system::error_code err;
    socket_server->connect(endpoints);
    tcp::no_delay option(true);
    socket_server->set_option(option);

    SocketsInfo sockets_info_gui(socket_gui_receive, gui_endpoint, socket_server);
    std::thread receive_from_gui_thread{receive_from_gui, std::ref(*arguments), std::ref(game_state),
                                        std::ref(sockets_info_gui)};

    SocketsInfo sockets_info_server(socket_gui_send, gui_endpoint, socket_server);
    std::thread receive_from_server_thread{receive_from_server, std::ref(*arguments), std::ref(game_state),
                                           std::ref(sockets_info_server)};

    receive_from_server_thread.join();
    return 0;
    receive_from_gui_thread.join();

    return 0;
}
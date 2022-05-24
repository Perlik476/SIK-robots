#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <utility>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <thread>

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


void receive_from_gui(Arguments &arguments, GameState &game_state, SocketsInfo &sockets_info,
                      ThreadsInfo &threads_info) {
    for (;;) {
        boost::array<char, 3> buffer{}; // TODO
        udp::endpoint remote_endpoint;
        size_t size;
        try {
            std::cerr << "waiting on receive_from" << std::endl;
            size = sockets_info.get_gui_socket()->receive_from(boost::asio::buffer(buffer), remote_endpoint);
            std::cerr << "got receive_from" << std::endl;
        }
        catch (std::exception &exception) {
            std::cerr << "Receiving from GUI failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;

            std::unique_lock lock(threads_info.get_mutex());
            threads_info.set_should_exit();
            return;
        }

        std::cout << "packet (size = " << size << "): ";
        for (size_t i = 0; i < size; i++) {
            std::cout << (int) buffer[i] << " ";
        }
        std::cout << "\n";

        if (size == 0) {
            continue;
        }

        Bytes bytes;
        try {
            bytes = Bytes(get_c_array(buffer), size);
        }
        catch (std::exception &exception) {
            std::cerr << "Deserialization of message from GUI failed." << std::endl;
            std::cerr << exception.what() << std::endl;
        }

        auto message = get_gui_message(bytes);
        if (message != nullptr && bytes.is_end()) {
            if (!game_state.is_joined) {
                try {
                    JoinMessage(arguments.player_name).send(sockets_info.get_server_socket());
                }
                catch (std::exception &exception) {
                    std::cerr << "Sending Join message to server failed. Terminating." << std::endl;
                    std::cerr << exception.what() << std::endl;
                    std::unique_lock lock(threads_info.get_mutex());
                    threads_info.set_should_exit();
                    return;
                }
                std::cout << "Join sent." << std::endl;
                game_state.is_joined = true;
            }
            else {
                try {
                    message->execute(game_state, sockets_info);
                }
                catch (std::exception &exception) {
                    std::cerr << "Sending message to server failed. Terminating." << std::endl;
                    std::cerr << exception.what() << std::endl;
                    std::unique_lock lock(threads_info.get_mutex());
                    threads_info.set_should_exit();
                    return;
                }
                std::cout << "message sent." << std::endl;
            }
        }
    }
}

void receive_from_server(Arguments &arguments, GameState &game_state, SocketsInfo sockets_info,
                         ThreadsInfo &threads_info) {
    for (;;) {
        BytesReceiver bytes;
        try {
            std::cout << "BYTES" << std::endl;
            bytes = BytesReceiver(sockets_info.get_server_socket());
        }
        catch (std::exception &exception) {
            std::cerr << "Receiving message from server failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;
            std::unique_lock lock(threads_info.get_mutex());
            threads_info.set_should_exit();
            return;
        }
        while (!bytes.is_end()) {
            std::shared_ptr<ServerMessage> message;
            try {
                std::cout << "MESSAGE" << std::endl;
                message = get_server_message(bytes);
            }
            catch (std::exception &exception) {
                std::cerr << "Receiving message from server failed. Terminating." << std::endl;
                std::cerr << exception.what() << std::endl;
                std::unique_lock lock(threads_info.get_mutex());
                threads_info.set_should_exit();
                return;
            }
            if (message == nullptr) {
                std::cout << "Message received from server could not be deserialized. Terminating." << std::endl;
                std::unique_lock lock(threads_info.get_mutex());
                threads_info.set_should_exit();
                return;
            }
            std::cout << "executing server message.\n";
            try {
                message->execute(game_state, sockets_info);
            }
            catch (std::exception &exception) {
                std::cerr << "Sending message to gui failed. Terminating." << std::endl;
                std::cerr << exception.what() << std::endl;
                std::unique_lock lock(threads_info.get_mutex());
                threads_info.set_should_exit();
                return;
            }
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
    udp::endpoint gui_endpoint;
    try {
        gui_endpoint = *resolver.resolve(arguments->gui_address_pure, arguments->gui_port).begin();
    }
    catch (std::exception &exception) {
        std::cerr << "Resolving GUI address failed." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    std::shared_ptr<udp::socket> gui_socket;
    try {
        gui_socket = std::make_shared<udp::socket>(io_context, udp::endpoint(udp::v6(), arguments->port));
    }
    catch (std::exception &exception) {
        std::cerr << "Socket for GUI communications could not be opened." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    tcp::resolver resolver_tcp(io_context);
    tcp::endpoint endpoints;
    try {
        endpoints = *resolver_tcp.resolve(arguments->server_address_pure, arguments->server_port);
    }
    catch (std::exception &exception) {
        std::cerr << "Resolving server address failed." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    std::shared_ptr<tcp::socket> server_socket;
    try {
        server_socket = std::make_shared<tcp::socket>(io_context);
        server_socket->connect(endpoints);
        tcp::no_delay option(true);
        server_socket->set_option(option);
    }
    catch (std::exception &exception) {
        std::cerr << "Connecting with server failed." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    ThreadsInfo threads_info;
    SocketsInfo sockets_info(gui_socket, gui_endpoint, server_socket);

    std::thread receive_from_gui_thread{receive_from_gui, std::ref(*arguments), std::ref(game_state),
                                        std::ref(sockets_info), std::ref(threads_info)};
    std::thread receive_from_server_thread{receive_from_server, std::ref(*arguments), std::ref(game_state),
                                           std::ref(sockets_info), std::ref(threads_info)};

    {
        std::unique_lock<std::mutex> lock(threads_info.get_mutex());
        while (!threads_info.get_should_exit()) {
            threads_info.get_condition_variable().wait(lock);
        }
        std::cerr << "main got termination" << std::endl;
    }

    boost::system::error_code error_code;

    std::cerr << "closing gui" << std::endl;
    sockets_info.get_gui_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
    sockets_info.get_gui_socket()->close(error_code);
    std::cerr << "closing server" << std::endl;
    sockets_info.get_server_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
    sockets_info.get_server_socket()->close(error_code);
    std::cerr << "gui join" << std::endl;

    receive_from_gui_thread.join();

    std::cerr << "GUI terminated." << std::endl;

    receive_from_server_thread.join();

    std::cerr << "Threads terminated." << std::endl;

    return 1;
}
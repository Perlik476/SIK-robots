#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <thread>

#include "definitions.h"
#include "messages.h"

namespace po = boost::program_options;

Arguments parse_arguments(int argc, char *argv[]) {
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

    return {player_name, port, gui_address, server_address};
}


void receive_from_gui(Arguments &arguments, GameState &game_state, SocketsInfo &sockets_info,
                      ThreadsInfo &threads_info) {
    for (;;) {
        boost::array<char, 3> buffer{}; // TODO
        udp::endpoint remote_endpoint;
        size_t size;
        try {
            size = sockets_info.get_gui_socket()->receive_from(boost::asio::buffer(buffer), remote_endpoint);
        }
        catch (std::exception &exception) {
            std::cerr << "Receiving from GUI failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;

            std::unique_lock lock(threads_info.get_mutex());
            threads_info.set_should_exit();
            return;
        }

//        std::cout << "packet (size = " << size << "): ";
//        for (size_t i = 0; i < size; i++) {
//            std::cout << (int) buffer[i] << " ";
//        }
//        std::cout << "\n";

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
//                std::cout << "Join sent." << std::endl;
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
//                std::cout << "message sent." << std::endl;
            }
        }
    }
}

void receive_from_server(GameState &game_state, SocketsInfo &sockets_info, ThreadsInfo &threads_info) {
    for (;;) {
        BytesReceiver bytes;
        try {
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
                std::cerr << "Message received from server could not be deserialized. Terminating." << std::endl;
                std::unique_lock lock(threads_info.get_mutex());
                threads_info.set_should_exit();
                return;
            }
//            std::cout << "executing server message.\n";
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
//    std::cout << "gui_address: " << arguments.gui_address << "\n"
//        << "server_address: " << arguments.server_address << "\n"
//        << "port: " << arguments.port << "\n"
//        << "player_name: " << arguments.player_name << "\n";

    if (!arguments.check_correctness()) {
        std::cerr << "Program arguments are invalid. Terminating." << std::endl;
        return 1;
    }

    auto game_state = GameState();

    ThreadsInfo threads_info;

    std::shared_ptr<SocketsInfo> sockets_info;
    try {
        sockets_info = std::make_shared<SocketsInfo>(arguments);
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        return 1;
    }

    std::thread gui_listener_thread{receive_from_gui, std::ref(arguments), std::ref(game_state),
                                    std::ref(*sockets_info), std::ref(threads_info)};
    std::thread server_listener_thread{receive_from_server, std::ref(game_state),
                                       std::ref(*sockets_info), std::ref(threads_info)};

    {
        std::unique_lock<std::mutex> lock(threads_info.get_mutex());
        while (!threads_info.get_should_exit()) {
            threads_info.get_condition_variable().wait(lock);
        }
        std::cerr << "Exception occurred in one of the threads." << std::endl;
    }

    boost::system::error_code error_code;

    std::cerr << "Closing GUI socket." << std::endl;
    sockets_info->get_gui_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
    sockets_info->get_gui_socket()->close(error_code);

    std::cerr << "Closing server socket." << std::endl;
    sockets_info->get_server_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
    sockets_info->get_server_socket()->close(error_code);

    gui_listener_thread.join();

    std::cerr << "GUI listener thread terminated." << std::endl;

    server_listener_thread.join();

    std::cerr << "Server listener thread terminated." << std::endl;

    std::cerr << "Client terminating." << std::endl;

    return 1;
}
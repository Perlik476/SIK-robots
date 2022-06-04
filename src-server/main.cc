#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <cstring>
#include <ranges>
#include <thread>

#include "includes.h"
#include "messages.h"


static bool try_receive_from_gui(Bytes &bytes, SocketsInfo &sockets_info, ThreadsInfo &threads_info) {
    boost::array<char, 3> buffer{};
    udp::endpoint endpoint;
    size_t size;
    try {
        size = sockets_info.get_gui_socket()->receive_from(boost::asio::buffer(buffer), endpoint);
    }
    catch (std::exception &exception) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Receiving from GUI failed: " << exception.what() << "Terminating." << std::endl;
        }

        std::unique_lock lock(threads_info.get_mutex());
        threads_info.set_should_exit();
        return false;
    }

    if (size == 0) {
        return true;
    }

    try {
        bytes = Bytes(get_c_array(buffer), size);
    }
    catch (std::exception &exception) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Message received from GUI could not be deserialized." << std::endl;
        }
    }

    return true;
}

//static bool try_send_to_server(std::shared_ptr<GuiMessage> &message, GameState &game_state,
//                               SocketsInfo &sockets_info, ThreadsInfo &threads_info) {
//    if (!game_state.get_is_joined()) {
//        try {
//            JoinMessage(game_state.get_player_name()).send(sockets_info.get_server_socket());
//        }
//        catch (std::exception &exception) {
//            if (!threads_info.get_should_exit()) {
//                std::cerr << "Sending Join message to server failed. Terminating." << std::endl;
//                std::cerr << exception.what() << std::endl;
//            }
//            std::unique_lock lock(threads_info.get_mutex());
//            threads_info.set_should_exit();
//            return false;
//        }
//        game_state.set_is_joined(true);
//    }
//    else {
//        try {
//            message->execute(game_state, sockets_info);
//        }
//        catch (std::exception &exception) {
//            if (!threads_info.get_should_exit()) {
//                std::cerr << "Sending message to server failed. Terminating." << std::endl;
//                std::cerr << exception.what() << std::endl;
//            }
//            std::unique_lock lock(threads_info.get_mutex());
//            threads_info.set_should_exit();
//            return false;
//        }
//    }
//
//    return true;
//}

//static void from_gui_to_server_communication(GameState &game_state, SocketsInfo &sockets_info,
//                                             ThreadsInfo &threads_info) {
//    for (;;) {
//        if (threads_info.get_should_exit()) {
//            return;
//        }
//
//        Bytes bytes;
//
//        if (!try_receive_from_gui(bytes, sockets_info, threads_info)) {
//            return;
//        }
//
//        std::shared_ptr<GuiMessage> message;
//        try {
//            message = get_gui_message(bytes);
//        }
//        catch (std::exception &exception) {
//            if (!threads_info.get_should_exit()) {
//                std::cerr << "Message received from GUI could not be deserialized." << std::endl;
//            }
//        }
//
//        if (threads_info.get_should_exit()) {
//            return;
//        }
//
//        if (message != nullptr && bytes.is_end()) {
//            if (!try_send_to_server(message, game_state, sockets_info, threads_info)) {
//                return;
//            }
//        }
//    }
//}

static bool try_create_bytes_receiver(BytesReceiver &bytes, SocketsInfo &sockets_info, ThreadsInfo &threads_info) {
    try {
        bytes = BytesReceiver(sockets_info.get_server_socket());
    }
    catch (std::exception &exception) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Receiving message from server failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;
        }
        std::unique_lock lock(threads_info.get_mutex());
        threads_info.set_should_exit();
        return false;
    }
    return true;
}

static bool try_process_message(GameState &game_state, BytesReceiver &bytes, SocketsInfo &sockets_info,
                                ThreadsInfo &threads_info) {
    std::shared_ptr<ServerMessage> message;

    try {
        message = get_server_message(bytes);
    }
    catch (std::exception &exception) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Receiving message from server failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;
        }
        std::unique_lock lock(threads_info.get_mutex());
        threads_info.set_should_exit();
        return false;
    }
    if (message == nullptr) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Message received from server could not be deserialized. Terminating." << std::endl;
        }
        std::unique_lock lock(threads_info.get_mutex());
        threads_info.set_should_exit();
        return false;
    }
    try {
        message->execute(game_state, sockets_info);
    }
    catch (std::exception &exception) {
        if (!threads_info.get_should_exit()) {
            std::cerr << "Sending message to gui failed. Terminating." << std::endl;
            std::cerr << exception.what() << std::endl;
        }
        std::unique_lock lock(threads_info.get_mutex());
        threads_info.set_should_exit();
        return false;
    }

    return true;
}


static void from_server_to_gui_communication(GameState &game_state, SocketsInfo &sockets_info,
                                             ThreadsInfo &threads_info) {
    for (;;) {
        if (threads_info.get_should_exit()) {
            return;
        }

        BytesReceiver bytes;
        if (!try_create_bytes_receiver(bytes, sockets_info, threads_info)) {
            return;
        }

        if (threads_info.get_should_exit()) {
            return;
        }

        while (!bytes.is_end()) {
            if (!try_process_message(game_state, bytes, sockets_info, threads_info)) {
                return;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    std::shared_ptr<Arguments> arguments;
    try {
        arguments = std::make_shared<Arguments>(argc, argv);
    }
    catch (std::exception &exception) {
        std::cerr << "Program arguments are invalid. Terminating." << std::endl;
        std::cerr << exception.what() << std::endl;
        return 1;
    }
    if (!arguments->check_correctness()) {
        std::cerr << "Program arguments are invalid. Terminating." << std::endl;
        return 1;
    }

    std::cout << "OK\n";

//    auto game_state = GameState(arguments->player_name);

//    ThreadsInfo threads_info;
//    std::shared_ptr<SocketsInfo> sockets_info;
//    try {
//        sockets_info = std::make_shared<SocketsInfo>(*arguments);
//    }
//    catch (std::exception &exception) {
//        std::cerr << exception.what() << std::endl;
//        return 1;
//    }
//
//    std::thread gui_listener_thread{from_gui_to_server_communication, std::ref(game_state),
//                                    std::ref(*sockets_info), std::ref(threads_info)};
//    std::thread server_listener_thread{from_server_to_gui_communication, std::ref(game_state),
//                                       std::ref(*sockets_info), std::ref(threads_info)};
//
//    {
//        std::unique_lock<std::mutex> lock(threads_info.get_mutex());
//        while (!threads_info.get_should_exit()) {
//            threads_info.get_condition_variable().wait(lock);
//        }
//        std::cerr << "Exception occurred in one of the threads." << std::endl;
//    }
//
//    boost::system::error_code error_code;
//
//    sockets_info->get_gui_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
//    sockets_info->get_gui_socket()->close(error_code);
//
//    sockets_info->get_server_socket()->shutdown(boost::asio::socket_base::shutdown_both, error_code);
//    sockets_info->get_server_socket()->close(error_code);
//
//    gui_listener_thread.join();
//    server_listener_thread.join();
//
//    std::cerr << "Client terminating." << std::endl;

    return 1;
}
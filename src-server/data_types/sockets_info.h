#ifndef ROBOTS_SOCKETS_INFO_H
#define ROBOTS_SOCKETS_INFO_H

#include "definitions.h"

class SocketsInfo {
    boost::asio::io_context io_context;
    gui_socket_t gui_socket;
    gui_endpoint_t gui_endpoint;
    socket_t server_socket;

public:
    SocketsInfo() = default;

    SocketsInfo(const std::string &address, const std::string &port) {
        tcp::resolver resolver_tcp(io_context);
        tcp::endpoint endpoint;
        try {
            endpoint = *resolver_tcp.resolve(address, port);
        }
        catch (...) {
            std::cerr << "Resolving server address failed." << std::endl;
            throw;
        }

        try {
            server_socket = std::make_shared<tcp::socket>(io_context);
            server_socket->connect(endpoint);
            tcp::no_delay option(true);
            server_socket->set_option(option);
        }
        catch (...) {
            std::cerr << "Connecting with server failed." << std::endl;
            throw;
        }
    }

    auto &get_gui_socket() {
        return gui_socket;
    }

    auto &get_server_socket() {
        return server_socket;
    }

    auto &get_gui_endpoint() {
        return gui_endpoint;
    }
};

#endif //ROBOTS_SOCKETS_INFO_H

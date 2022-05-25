#ifndef ROBOTS_SOCKETS_INFO_H
#define ROBOTS_SOCKETS_INFO_H

#include "usings.h"

class SocketsInfo {
    boost::asio::io_context io_context;
    gui_socket_t gui_socket;
    gui_endpoint_t gui_endpoint;
    server_socket_t server_socket;

public:
    SocketsInfo() = default;

    SocketsInfo(Arguments &arguments) {
        udp::resolver resolver(io_context);
        try {
            gui_endpoint = *resolver.resolve(arguments.gui_address_pure, arguments.gui_port).begin();
        }
        catch (std::exception &exception) {
            std::cerr << "Resolving GUI address failed." << std::endl;
            throw exception;
        }

        try {
            gui_socket = std::make_shared<udp::socket>(io_context, udp::endpoint(udp::v6(), arguments.port));
        }
        catch (std::exception &exception) {
            std::cerr << "Socket for GUI communications could not be opened." << std::endl;
            throw exception;
        }

        tcp::resolver resolver_tcp(io_context);
        tcp::endpoint endpoints;
        try {
            endpoints = *resolver_tcp.resolve(arguments.server_address_pure, arguments.server_port);
        }
        catch (std::exception &exception) {
            std::cerr << "Resolving server address failed." << std::endl;
            throw exception;
        }

        try {
            server_socket = std::make_shared<tcp::socket>(io_context);
            server_socket->connect(endpoints);
            tcp::no_delay option(true);
            server_socket->set_option(option);
        }
        catch (std::exception &exception) {
            std::cerr << "Connecting with server failed." << std::endl;
            throw exception;
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

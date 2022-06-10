#ifndef ROBOTS_DEFINITIONS_H
#define ROBOTS_DEFINITIONS_H

#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include <utility>
#include <cstdint>
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <variant>
#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <set>
#include <mutex>
#include <condition_variable>

using bytes_t = char *;
using tcp = boost::asio::ip::tcp;
using socket_t = std::shared_ptr<tcp::socket>;

class Bytes;
class BytesReceiver;

class GameState;

class Serializable {
public:
    virtual Bytes serialize() const = 0;

    virtual ~Serializable() = default;
};

class ClientState;

class Executable {
public:
    virtual ~Executable() = default;

    virtual void execute(std::shared_ptr<GameState> &game_state,
                         std::shared_ptr<ClientState> &client) = 0;
};

template<class T>
concept isSerializable = std::is_base_of_v<Serializable, T>;

template<class T>
concept isExecutable = std::is_base_of_v<Executable, T>;

enum Direction: char {
    Up,
    Right,
    Down,
    Left,
    Undefined
};

Direction get_direction(Bytes &bytes);

class String;
String socket_to_string(socket_t &socket);

#endif //ROBOTS_DEFINITIONS_H

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
using udp = boost::asio::ip::udp;
using tcp = boost::asio::ip::tcp;
using socket_t = std::shared_ptr<tcp::socket>;
using gui_socket_t = std::shared_ptr<udp::socket>;
using gui_endpoint_t = udp::endpoint;

class Bytes;
class BytesReceiver;

class GameState;

class Serializable {
public:
    virtual Bytes serialize() const = 0;

    virtual ~Serializable() = default;
};

class SocketsInfo;

class Executable {
public:
    virtual ~Executable() = default;

    virtual void execute([[maybe_unused]] GameState &game_state) = 0;
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

#endif //ROBOTS_DEFINITIONS_H

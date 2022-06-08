#include "definitions.h"
#include "bytes.h"
#include "string.h"

Direction get_direction(Bytes &bytes) {
    char c = bytes.get_next_byte();
    if (c >= 4) {
        throw BytesDeserializationException();
    }
    return (Direction) c;
}

String socket_to_string(socket_t &socket) {
    auto endpoint = socket->remote_endpoint();
    std::stringstream ss;
//    ss << "[" << endpoint.address().to_string() << "]:" << endpoint.port();
    ss << endpoint;
    return String(ss.str());
}
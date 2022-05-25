#ifndef ROBOTS_BYTES_H
#define ROBOTS_BYTES_H

#include "usings.h"

class BytesDeserializationException : public std::exception {
public:
    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
        return "Message deserialization failed.";
    }
};

class Bytes {
protected:
    std::vector<char> vector;

    size_t index = 0;

    void processed(size_t number_of_bytes) {
        index += number_of_bytes;
    }
public:
    Bytes() = default;

    Bytes(bytes_t bytes, size_t length);

    Bytes(std::vector<char> &vec);

    Bytes(std::string &string);

    Bytes(const std::string &string);

    Bytes(char byte);

    Bytes &operator+= (const Bytes &other);

    Bytes operator+ (const Bytes &other);

    virtual char get_next_byte() {
        if (is_end()) {
            throw BytesDeserializationException();
        }
        char value = vector[index];
        processed(1);
        return value;
    }

    std::vector<char> get_next_bytes(size_t n) {
        std::vector<char> bytes;
        for (size_t i = 0; i < n; i++) {
            bytes.push_back(get_next_byte());
        }
        return bytes;
    }

    bool is_end() {
        return index >= vector.size();
    }

    const auto &get_vector() const {
        return vector;
    }
};

class BytesReceiver: public Bytes {
    server_socket_t socket = nullptr;

    void listen() {
        boost::array<char, 128> buffer{};
        size_t size = socket->read_some(boost::asio::buffer(buffer));
        for (size_t i = 0; i < size; i++) {
            vector.push_back(buffer[i]);
        }
    }
public:
    BytesReceiver() = default;

    BytesReceiver(server_socket_t &socket) : socket(socket) {
        listen();
    }

    char get_next_byte() override {
        if (is_end()) {
            listen();
        }
        char value = vector[index];
        processed(1);
        return value;
    }
};

#endif //ROBOTS_BYTES_H

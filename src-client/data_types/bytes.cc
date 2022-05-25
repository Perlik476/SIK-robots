#include "bytes.h"

Bytes::Bytes(bytes_t bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        vector.push_back(bytes[i]);
    }
}

Bytes::Bytes(std::vector<char> &vec) {
    for (auto byte : vec) {
        vector.push_back(byte);
    }
}

Bytes::Bytes(std::string &string) {
    for (auto byte : string) {
        vector.push_back(byte);
    }
}

Bytes::Bytes(const std::string &string) {
    for (auto byte : string) {
        vector.push_back(byte);
    }
}

Bytes::Bytes(char byte) {
    vector.push_back(byte);
}

Bytes &Bytes::operator+= (const Bytes &other) {
    for (auto byte : other.vector) {
        vector.push_back(byte);
    }
    return *this;
}

Bytes Bytes::operator+ (const Bytes &other) {
    return *this += other;
}
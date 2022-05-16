#include "definitions.h"

Bytes::Bytes(bytes_t bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        push_back(bytes[i]);
    }
}

Bytes::Bytes(std::string &string) {
    for (auto byte : string) {
        push_back(byte);
    }
}

Bytes::Bytes(char byte) {
    push_back(byte);
}

Bytes &Bytes::operator+= (const Bytes &other) {
    for (auto byte : other) {
        push_back(byte);
    }
    return *this;
}

Bytes Bytes::operator+ (const Bytes &other) {
    return *this += other;
}


#include "definitions.h"

Bytes::Bytes(bytes_t bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        push_back(bytes[i]);
    }
}

Bytes::Bytes(std::vector<char> &vec) {
    for (auto byte : vec) {
        push_back(byte);
    }
}

Bytes::Bytes(std::string &string) {
    for (auto byte : string) {
        push_back(byte);
    }
}

Bytes::Bytes(const std::string &string) {
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
//
//Position &Position::operator+= (const Position &other) {
//    this->x.value += other.x.value;
//    this->y.value += other.y.value;
//    return *this;
//}
//
//Position Position::operator+ (const Position &other) {
//    return *this += other;
//}

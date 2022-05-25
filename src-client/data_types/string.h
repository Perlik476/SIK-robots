#ifndef ROBOTS_STRING_H
#define ROBOTS_STRING_H

#include "usings.h"
#include "bytes.h"
#include "uint.h"

class String: public Serializable {
    std::string string;
public:
    auto &get_string() {
        return string;
    }

    auto &get_string() const {
        return string;
    }

    String() = default;

    explicit String(const std::string &string) : string(string) {}

    String(const String &string) : string(string.get_string()) {}

    explicit String(Bytes &bytes) {
        Uint8 length = Uint8(bytes);
        for (size_t i = 0; i < length.get_value(); i++) {
            string += bytes.get_next_byte();
        }
    }

    String &operator= (const String &other) {
        this->string = other.get_string();
        return *this;
    }

    Bytes serialize() const override {
        auto length = static_cast<uint8_t>(string.length());

        return Uint8(length).serialize() + Bytes(string);
    }
};

#endif //ROBOTS_STRING_H

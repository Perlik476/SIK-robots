#ifndef ROBOTS_UINT_H
#define ROBOTS_UINT_H

#include "usings.h"
#include "bytes.h"

template<class T>
concept isUint = std::is_same_v<uint8_t, T> || std::is_same_v<uint16_t, T> || std::is_same_v<uint32_t, T>;

template<isUint T>
class Uint: public Serializable {
    T value;
public:
    Uint() = default;

    constexpr Uint(T value): value(value) {}

    explicit Uint(Bytes &bytes) {
        if constexpr (std::is_same_v<T, uint8_t>) {
            memcpy(&value, &(bytes.get_next_bytes(1)[0]), 1);
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            memcpy(&value, &(bytes.get_next_bytes(2)[0]), 2);
            value = ntohs(value);
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            memcpy(&value, &(bytes.get_next_bytes(4)[0]), 4);
            value = ntohl(value);
        }
    }

    Bytes serialize() const override {
        if constexpr (std::is_same_v<T, uint8_t>) {
            return { (char) value };
        }
        else if constexpr (std::is_same_v<T, uint16_t>) {
            char buffer[2];
            T value_to_net = htons(value);
            memcpy(buffer, &value_to_net, 2);
            return { buffer, 2 };
        }
        else if constexpr (std::is_same_v<T, uint32_t>) {
            char buffer[4];
            T value_to_net = htonl(value);
            memcpy(buffer, &value_to_net, 4);
            return { buffer, 4 };
        }
    }

    bool operator<(Uint<T> const &other) const {
        return value < other.get_value();
    }

    bool operator==(Uint<T> const &other) const {
        return value == other.get_value();
    }

    Uint<T> &operator+=(Uint<T> const &other) {
        value += other.get_value();
        return *this;
    }

    Uint<T> operator+(Uint<T> const &other) const {
        return *this += other;
    }

    Uint<T> &operator-=(Uint<T> const &other) {
        value -= other.get_value();
        return *this;
    }

    Uint<T> operator-(Uint<T> const &other) const {
        return *this += other;
    }

    auto get_value() {
        return value;
    }

    auto get_value() const {
        return value;
    }
};

using Uint8 = Uint<uint8_t>;
using Uint16 = Uint<uint16_t>;
using Uint32 = Uint<uint32_t>;


#endif //ROBOTS_UINT_H

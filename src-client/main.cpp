#include <iostream>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <utility>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <ranges>

namespace po = boost::program_options;

class Arguments {
public:
    std::string player_name;
    uint16_t port;
    std::string gui_address;
    std::string server_address;

public:
    Arguments(std::string player_name, uint16_t port, std::string gui_address, std::string server_address)
        : player_name(std::move(player_name)), port(port), gui_address(std::move(gui_address)),
        server_address(std::move(server_address)) {}
};

std::shared_ptr<Arguments> parse_arguments(int argc, char *argv[]) {
    std::string gui_address, player_name, server_address;
    uint16_t port;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("gui-address,d", po::value<std::string>(&gui_address))
            ("help,h", "Produce help message")
            ("player-name,n", po::value<std::string>(&player_name))
            ("port,p", po::value<uint16_t>(&port), "Port for GUI communication")
            ("server-address,s", po::value<std::string>(&server_address));

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    return make_shared<Arguments>(player_name, port, gui_address, server_address);
}

using bytes_t = char *;

class Bytes: std::vector<char> {
public:
    std::vector<char> vector;

    Bytes() = default;

    Bytes(bytes_t bytes, size_t length) {
        for (size_t i = 0; i < length; i++) {
            vector.push_back(bytes[i]);
        }
    }

    Bytes(const std::string &string) {
        for (auto byte : string) {
            vector.push_back(byte);
        }
    }

    Bytes &operator+= (const Bytes &other) {
        for (auto byte : other.vector) {
            vector.push_back(byte);
        }
        return *this;
    }

    Bytes operator+ (const Bytes &other) {
        return *this += other;
    }
};

class Serializable {
public:
    virtual Bytes serialize() = 0;
};

class String: public Serializable {
public:
    std::string string;

//    std::string() = default;

//    std::string(const std::string &value): value(value) {}

    Bytes serialize() override {
        char buffer[1];
        auto length = static_cast<uint8_t>(string.length()); // TODO check length < 256
        std::cout << "len: " << (int) length << "\n";
        memcpy(buffer, &length, 1);

        return Bytes(buffer, 1) + string;
    }
};

template<class T>
concept isSerializable = std::is_base_of_v<Serializable, T>;

template<isSerializable T>
class List: public Serializable {
public:
    std::vector<std::shared_ptr<T>> list;

//    List() = default;

    Bytes serialize() override {
        char buffer[4];
        auto length = static_cast<uint32_t>(list.size());
        length = htonl(length);
        memset(buffer, 0, 4);
        memcpy(buffer, &length, 4);

        Bytes list_content;
        for (auto &element : list) {
            list_content += element->serialize(); // TODO += stringów bad
        }

        return Bytes(buffer, 4) + list_content;
    }
};

class ClientMessage {
private:
    virtual Bytes serialize() = 0;

public:
    void send() {
        Bytes message = serialize();
    }
};

class JoinMessage : public ClientMessage {
private:
    std::string name;

    Bytes serialize() override {

    }

public:
};

int main(int argc, char *argv[]) {

    auto arguments = parse_arguments(argc, argv);
    std::cout << "gui_address: " << arguments->gui_address << "\n"
        << "server_address: " << arguments->server_address << "\n"
        << "port: " << arguments->port << "\n"
        << "player_name: " << arguments->player_name << "\n";

    List<String> list;
    auto string1 = std::make_shared<String>();
    string1->string = "lol";
    auto string2 = std::make_shared<String>();
    string2->string = "XDD";
    list.list.push_back(string1);
    list.list.push_back(string2);

    auto bytes = list.serialize();
    uint32_t len;
    memcpy(&len, &bytes.vector.front(), 4);
    len = ntohl(len);
    std::cout << len << " " << bytes.vector.size() << "\n";

    return 0;
}
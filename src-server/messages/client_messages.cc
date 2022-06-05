#include "client_messages.h"
#include "../threads.h"

void JoinMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientInfo> &client_info) {
    std::cout << "Join: " << name.get_string() << std::endl;

    auto endpoint = client_info->get_socket()->remote_endpoint();
    std::stringstream ss;
    ss << "[" << endpoint.address().to_string() << "]:" << endpoint.port();
    game_state->try_add_player(name, String(ss.str()));
    HelloMessage(game_state).send(client_info->get_socket());

    auto sender_thread = std::make_shared<std::thread>(sender_fun, client_info, std::ref(game_state));

    sender_thread->detach();
}
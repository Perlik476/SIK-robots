#include "client_messages.h"
#include "../threads.h"
#include "../includes.h"

void JoinMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientInfo> &client_info) {
    std::cout << "Join: " << name.get_string() << std::endl;

    game_state->try_add_player(name, socket_to_string(client_info->get_socket()));
    HelloMessage(game_state).send(client_info->get_socket());

    auto sender_thread = std::make_shared<std::thread>(sender_fun, client_info, std::ref(game_state));

    sender_thread->detach();
}

void MoveMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientInfo> &client_info) {
    std::cout << "Move: " << direction << std::endl;
    std::shared_ptr<Action> action = std::make_shared<MoveAction>(direction);
    game_state->set_action(action, client_info->get_socket());
}

void PlaceBombMessage::execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] std::shared_ptr<ClientInfo> &client_info) {
    std::cout << "PlaceBomb" << std::endl;
    std::shared_ptr<Action> action = std::make_shared<PlaceBombAction>();
    game_state->set_action(action, client_info->get_socket());
}

void PlaceBlockMessage::execute(std::shared_ptr<GameState> &game_state, [[maybe_unused]] std::shared_ptr<ClientInfo> &client_info) {
    std::cout << "PlaceBlock" << std::endl;
    std::shared_ptr<Action> action = std::make_shared<PlaceBlockAction>();
    game_state->set_action(action, client_info->get_socket());
}
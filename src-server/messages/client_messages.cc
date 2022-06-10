#include "client_messages.h"
#include "../threads.h"
#include "../includes.h"

void JoinMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) {
    game_state->try_add_player(name, socket_to_string(client_info->get_socket()));
}

void MoveMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) {
    std::shared_ptr<Action> action = std::make_shared<MoveAction>(direction);
    game_state->set_action(action, client_info->get_socket());
}

void PlaceBombMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) {
    std::shared_ptr<Action> action = std::make_shared<PlaceBombAction>();
    game_state->set_action(action, client_info->get_socket());
}

void PlaceBlockMessage::execute(std::shared_ptr<GameState> &game_state, std::shared_ptr<ClientState> &client_info) {
    std::shared_ptr<Action> action = std::make_shared<PlaceBlockAction>();
    game_state->set_action(action, client_info->get_socket());
}
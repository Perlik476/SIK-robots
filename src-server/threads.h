#ifndef ROBOTS_THREADS_H
#define ROBOTS_THREADS_H

#include "includes.h"

class ClientState;

void acceptor_fun(std::shared_ptr<GameState> &game_state, boost::asio::io_context &io_context,
                  tcp::acceptor &acceptor);

void game_loop(std::shared_ptr<GameState> &game_state);

void receiver_fun(std::shared_ptr<ClientState> client, std::shared_ptr<GameState> &game_state,
                  std::atomic_int &current_connections);

#endif //ROBOTS_THREADS_H

#ifndef ROBOTS_THREADS_H
#define ROBOTS_THREADS_H

#include "includes.h"

class ClientInfo;

void sender_fun(std::shared_ptr<ClientInfo> client_info, std::shared_ptr<GameState> &game_state);

void receiver_fun(std::shared_ptr<ClientInfo> client_info, std::shared_ptr<GameState> &game_state,
                  std::atomic_int &current_connections);

#endif //ROBOTS_THREADS_H

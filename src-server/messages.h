#ifndef ROBOTS_MESSAGES_H
#define ROBOTS_MESSAGES_H

#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "includes.h"
#include "messages/client_messages.h"
#include "messages/server_messages.h"

class ClientMessage;
class ServerMessage;

std::shared_ptr<ClientMessage> get_client_message(Bytes &bytes);

#endif // ROBOTS_MESSAGES_H
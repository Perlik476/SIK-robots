#ifndef ROBOTS_MESSAGES_H
#define ROBOTS_MESSAGES_H

#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "includes.h"
#include "messages/client_messages.h"
#include "messages/draw_messages.h"
#include "messages/gui_messages.h"
#include "messages/server_messages.h"

class ClientMessage;

class DrawMessage;

class ServerMessage;

class GuiMessage;

std::shared_ptr<ServerMessage> get_server_message(Bytes &bytes);

std::shared_ptr<GuiMessage> get_gui_message(Bytes &bytes);\

#endif // ROBOTS_MESSAGES_H
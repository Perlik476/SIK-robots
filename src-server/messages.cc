#include "messages.h"
#include "messages/server_messages.h"

std::shared_ptr<ServerMessage> get_server_message(Bytes &bytes) {
    char type = bytes.get_next_byte();
    switch(type) {
        case ServerMessageType::Hello:
            return std::make_shared<HelloMessage>(bytes);
        case ServerMessageType::AcceptedPlayer:
            return std::make_shared<AcceptedPlayerMessage>(bytes);
        case ServerMessageType::GameStarted:
            return std::make_shared<GameStartedMessage>(bytes);
        case ServerMessageType::Turn:
            return std::make_shared<TurnMessage>(bytes);
        case ServerMessageType::GameEnded:
            return std::make_shared<GameEndedMessage>(bytes);
        default:
            return nullptr;
    }
}
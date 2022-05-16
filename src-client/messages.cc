#include "messages.h"

std::shared_ptr<ServerMessage> get_server_message(Bytes &bytes) {
    char type = bytes.get_next_byte();
    switch(type) {
        case ServerMessageType::Hello:
            return std::make_shared<HelloMessage>(bytes);
            break;
        case ServerMessageType::AcceptedPlayer:
            return std::make_shared<AcceptedPlayerMessage>(bytes);
            break;
        case ServerMessageType::GameStarted:
            return std::make_shared<GameStartedMessage>(bytes);
            break;
        case ServerMessageType::Turn:
            return std::make_shared<TurnMessage>(bytes);
            break;
        case ServerMessageType::GameEnded:
            return std::make_shared<GameEndedMessage>(bytes);
            break;
        default:
            break;
    }
}
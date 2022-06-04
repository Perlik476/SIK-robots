#include "messages.h"
#include "messages/server_messages.h"

std::shared_ptr<ServerMessage> get_message(Bytes &bytes) {
    char type = bytes.get_next_byte();
    switch(type) {
        case ClientMessageType::Join:
//            return std::make_shared<HelloMessage>(bytes);
        case ClientMessageType::PlaceBomb:
//            return std::make_shared<AcceptedPlayerMessage>(bytes);
        case ClientMessageType::PlaceBlock:
//            return std::make_shared<GameStartedMessage>(bytes);
        case ClientMessageType::Move:
//            return std::make_shared<TurnMessage>(bytes);
        default:
            return nullptr;
    }
}
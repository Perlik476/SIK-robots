#include "messages.h"
#include "messages/client_messages.h"

std::shared_ptr<ClientMessage> get_client_message(Bytes &bytes) {
    char type = bytes.get_next_byte();
    switch(type) {
        case ClientMessageType::Join:
            return std::make_shared<JoinMessage>(bytes);
        case ClientMessageType::PlaceBomb:
            return std::make_shared<PlaceBombMessage>();
        case ClientMessageType::PlaceBlock:
            return std::make_shared<PlaceBlockMessage>();
        case ClientMessageType::Move:
            return std::make_shared<MoveMessage>(bytes);
        default:
            return nullptr;
    }
}
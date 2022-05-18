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
            return nullptr;
            break;
    }
}

//std::shared_ptr<GuiMessage> get_gui_message(Bytes &&bytes) {
//    get_gui_message(std::move(bytes));
//}

std::shared_ptr<GuiMessage> get_gui_message(Bytes &bytes) {
    char type = bytes.get_next_byte();
    switch(type) {
        case GuiMessageType::PlaceBlock:
            return std::make_shared<PlaceBlockGuiMessage>();
            break;
        case GuiMessageType::PlaceBomb:
            return std::make_shared<PlaceBombGuiMessage>();
            break;
        case GuiMessageType::Move:
            return std::make_shared<MoveGuiMessage>(bytes);
            break;
        default:
            return nullptr;
            break;
    }
}
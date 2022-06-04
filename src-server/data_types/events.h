//#ifndef ROBOTS_EVENTS_H
//#define ROBOTS_EVENTS_H
//
//#include "definitions.h"
//#include "bytes.h"
//#include "uint.h"
//#include "position.h"
//#include "bomb.h"
//#include "player.h"
//#include "list.h"
//
//class BombPlacedEvent: public Serializable {
//    BombId id;
//    Position position;
//
//public:
//    explicit BombPlacedEvent(Bytes &bytes) {
//        id = BombId(bytes);
//        position = Position(bytes);
//    }
//
////    void execute(GameState &game_state, [[maybe_unused]] SocketsInfo &sockets_info) override;
//};
//
//class BombExplodedEvent: public Serializable {
//    BombId id;
//    List<player_id_t> robots_destroyed;
//    List<Position> blocks_destroyed;
//
//    void remove_bomb(GameState &game_state, std::shared_ptr<Bomb> &bomb);
//
//    void set_dead_players(GameState &game_state);
//
//    void remove_blocks(GameState &game_state);
//
//    bool is_explosion_inside_a_block(GameState &game_state, Position &bomb_position);
//
//    void add_explosions(GameState &game_state, Position &bomb_position);
//public:
//    explicit BombExplodedEvent(Bytes &bytes) {
//        id = BombId(bytes);
//        robots_destroyed = List<player_id_t>(bytes);
//        blocks_destroyed = List<Position>(bytes);
//    }
//};
//
//class PlayerMovedEvent: public Serializable {
//    player_id_t id;
//    Position position;
//
//public:
//    explicit PlayerMovedEvent(Bytes &bytes) {
//        id = player_id_t(bytes);
//        position = Position(bytes);
//    }
//};
//
//class BlockPlacedEvent: public Serializable {
//    Position position;
//
//public:
//    explicit BlockPlacedEvent(Bytes &bytes) {
//        position = Position(bytes);
//    }
//};
//
//class Event: public Serializable {
//    enum Type: char {
//        BombPlaced,
//        BombExploded,
//        PlayerMoved,
//        BlockPlaced,
//    } type;
//
//    using event_t = std::variant<BombPlacedEvent, BombExplodedEvent,
//            PlayerMovedEvent, BlockPlacedEvent>;
//
//    event_t event;
//
//    event_t get_event(Bytes &bytes);
//public:
//    explicit Event(Bytes &bytes) : event(get_event(bytes)) {}
//};
//
//#endif //ROBOTS_EVENTS_H

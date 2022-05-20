//bool is_block_on_position(const Position &position) {
//    for (const auto &block_position : blocks.list) {
//        if (block_position->x.value == position.x.value
//            && block_position->y.value == position.y.value) {
//            return true;
//        }
//    }
//    return false;
//}
//
//void place_bomb(Position position) {
//    bombs.list.push_back(std::make_shared<Bomb>(
//            position, bomb_timer));
//}
//
//void place_block(Position position) {
//    if (!is_block_on_position(position)) {
//        blocks.list.push_back(std::make_shared<Position>(position.x, position.y));
//    }
//}
//
//void try_move(Direction direction, const std::shared_ptr<Position> &position) {
//    switch(direction) {
//        case Direction::Down:
//            if (!is_block_on_position(position->down()) && position->y.value > 0) {
//                position->y.value--;
//            }
//            break;
//        case Direction::Left:
//            if (!is_block_on_position(position->left()) && position->x.value > 0) {
//                position->x.value--;
//            }
//            break;
//        case Direction::Up:
//            if (!is_block_on_position(position->up()) && position->y.value < size_y.value - 1) {
//                position->y.value++;
//            }
//            break;
//        case Direction::Right:
//            if (!is_block_on_position(position->right()) && position->x.value < size_x.value - 1) {
//                position->x.value++;
//            }
//            break;
//        default:
//            // TODO
//            break;
//    }
//}
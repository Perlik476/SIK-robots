#include "arguments.h"

namespace po = boost::program_options;

void Arguments::parse_arguments(int argc, char *argv[]) {
    uint16_t players_count_temp;

    po::options_description desc("Allowed options");
    desc.add_options()
            ("bomb-timer,b", po::value<uint16_t>(&bomb_timer)->required())
            ("players-count,c", po::value<uint16_t>(&players_count_temp)->required())
            ("turn-duration,d", po::value<uint64_t>(&turn_duration)->required())
            ("explosion-radius,e", po::value<uint16_t>(&explosion_radius)->required())
            ("help,h", "Produce help message")
            ("initial-blocks,k", po::value<uint16_t>(&initial_blocks)->required())
            ("game-length,l", po::value<uint16_t>(&game_length)->required())
            ("server-name,n", po::value<std::string>(&server_name)->required())
            ("port,p", po::value<uint16_t>(&port)->required())
            ("seed,s", po::value<uint32_t>(&seed))
            ("size-x,x", po::value<uint16_t>(&size_x)->required())
            ("size-y,y", po::value<uint16_t>(&size_y)->required());

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    po::notify(vm);

    if (players_count_temp > UINT8_MAX) {
        // TODO
    }

    players_count = static_cast<uint8_t>(players_count_temp);
}
#include <cstdio>
#include <cstring>

#include "runtime.h"

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 ||
            std::strcmp(argv[i], "-h") == 0) {
            std::printf(
                "SuperMarioWorldRecomp [--bios <path>] [--rom <path>] [game.toml]\n");
            return 0;
        }
    }

    gbarecomp::RunOptions opts;
    opts.builtin_game_name = "Super Mario Advance 2: Super Mario World";
    opts.builtin_rom_sha1 = "5101ddf223d1d918928fe1f306b63a42ada14a5e";
    return gbarecomp::run_game(argc, argv, opts);
}

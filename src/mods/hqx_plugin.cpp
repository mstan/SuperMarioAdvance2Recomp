#include "mod_runtime.h"
#include "hq2x_filter.h"
#include <cstring>

namespace {
void reset_smoothing() { (void)gbarecomp::set_hqx_scale(0); }
void activate_smoothing() {
    const char* choice = gba_mod_option_value(
        "super-mario-world.enhancement.hq2x", "hq2x", "filter");
    // A saved legacy HQ2x toggle has no option: retain its original filter.
    const unsigned scale = choice && std::strcmp(choice, "hq3x") == 0 ? 3 :
                           choice && std::strcmp(choice, "hq4x") == 0 ? 4 : 2;
    (void)gbarecomp::set_hqx_scale(scale);
}
}
GBA_MOD_CONSTRUCTOR(smw_register_pixel_smoothing) {
    (void)gba_mod_register_reset_callback(reset_smoothing);
    (void)gba_mod_register_activation_plugin(
        "super-mario-world.pixel-smoothing", activate_smoothing);
}

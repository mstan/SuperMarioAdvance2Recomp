#include "smw_extended_view.h"

#include <cstdio>
#include <cstdlib>

#include "gba_bus.h"
#include "gba_ppu.h"
#include "runtime_bus_bridge.h"

extern "C" unsigned g_ws_extra_left;

namespace smw {
namespace {

int (*g_previous_tilemap_provider)(int, int, int, std::uint16_t*) = nullptr;
int (*g_previous_bg_x_provider)(int, int, int, int*) = nullptr;

bool title_layout(const std::uint8_t* io) {
    const std::uint16_t dispcnt = static_cast<std::uint16_t>(
        io[0] | (static_cast<std::uint16_t>(io[1]) << 8));
    return dispcnt == 0x1600u;
}

void trace_scene_once() {
    static unsigned long long last_frame = ~0ull;
    if (last_frame == g_runtime_vblank_starts) return;
    last_frame = g_runtime_vblank_starts;
    const char* trace = std::getenv("GBARECOMP_SMW_WS_TRACE");
    if (!trace || !*trace || *trace == '0') return;
    static const unsigned long long target = std::strtoull(trace, nullptr, 0);
    if (target > 1 && target != g_runtime_vblank_starts) return;
    gba::GbaBus* bus = gbarecomp::active_bus();
    if (!bus) return;
    const std::uint8_t* io = bus->io().raw();
    auto read16 = [&](unsigned offset) {
        return static_cast<unsigned>(io[offset] |
            (static_cast<unsigned>(io[offset + 1]) << 8));
    };
    std::fprintf(stderr,
        "[smw:extended-view] frame=%llu DISPCNT=%04X "
        "BG=%04X/%04X/%04X/%04X WIN=%04X/%04X/%04X/%04X\n",
        g_runtime_vblank_starts, read16(0x00), read16(0x08),
        read16(0x0A), read16(0x0C), read16(0x0E), read16(0x40),
        read16(0x42), read16(0x48), read16(0x4A));
}

int streamed_tilemap_provider(int bg, int hw_x, int screen_y,
                              std::uint16_t* out_entry) {
    trace_scene_once();
    if (gba::GbaBus* bus = gbarecomp::active_bus()) {
        // The title uses wrapped sprite pieces just outside native X as part
        // of the centered logo. Expanding that staging area exposes garbage,
        // so retain a faithful centered title and reopen the margins as soon
        // as the game switches to a scrolling map/gameplay layout.
        gba::g_ws_pillarbox = title_layout(bus->io().raw()) ? 1 : 0;
    }
    // SMA2 maintains its scrolling regular backgrounds as hardware tile maps
    // and streams new rows/columns through UpdateVramTileMaps (08001838).
    // Explicitly accepting the renderer's computed map entry also authorizes
    // those background layers outside native WIN0/WIN1 rectangles.
    if (bg >= 0 && bg < 4 && (hw_x < 0 || hw_x >= 240)) {
        return gba::kWsTilemapKeepWrapped;
    }
    return g_previous_tilemap_provider
        ? g_previous_tilemap_provider(bg, hw_x, screen_y, out_entry)
        : gba::kWsTilemapUnavailable;
}

int bounded_bg_x(int bg, int output_x, int screen_y, int* out_hw_x) {
    auto fallback = [&]() {
        return g_previous_bg_x_provider
            ? g_previous_bg_x_provider(bg, output_x, screen_y, out_hw_x) : 0;
    };
    if (!out_hw_x || bg < 0 || bg >= 4) return fallback();

    gba::GbaBus* bus = gbarecomp::active_bus();
    if (!bus) return fallback();
    const std::uint8_t* io = bus->io().raw();
    const unsigned cnt_offset = 0x08u + static_cast<unsigned>(bg) * 2u;
    const std::uint16_t bgcnt = static_cast<std::uint16_t>(
        io[cnt_offset] | (static_cast<std::uint16_t>(io[cnt_offset + 1]) << 8));

    // A 256-pixel-wide BG has no room for a complete 24-pixel margin on both
    // sides. Sampling beyond its ring can leak stale tiles (the title screen's
    // left edge is the visible example), so extend its authentic edge pixel.
    // 512-pixel maps retain the normal off-screen continuation.
    if ((bgcnt & 0x4000u) == 0) {
        const int left = static_cast<int>(g_ws_extra_left);
        if (output_x < left) {
            *out_hw_x = 0;
            return 1;
        }
        if (output_x >= left + 240) {
            *out_hw_x = 239;
            return 1;
        }
    }
    return fallback();
}

}  // namespace

void install_extended_view(std::uint32_t, std::uint32_t) {
    g_previous_tilemap_provider = gba::g_ws_tilemap_provider;
    g_previous_bg_x_provider = gba::g_ws_bg_x_provider;
    gba::g_ws_tilemap_provider = streamed_tilemap_provider;
    gba::g_ws_bg_x_provider = bounded_bg_x;
    gba::g_ws_authored_margin_layers = 1;
    gba::g_ws_pillarbox = 0;
    gba::g_ws_pillarbox_left = 0;
    gba::g_ws_pillarbox_right = 0;
}

}  // namespace smw

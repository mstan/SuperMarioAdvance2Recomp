#include "mod_runtime.h"
#include "sha1.h"
#include "smw_restoration.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
struct Activation { const char* id; GBAModActivationCallback callback; } activations[8];
unsigned activation_count, asset_requests;
GBAModActivationCallback reset_callback;
}

extern "C" int gba_mod_register_activation_plugin(const char* id, GBAModActivationCallback callback) {
    if (activation_count == 8) std::abort();
    activations[activation_count++] = {id, callback};
    return 1;
}
extern "C" int gba_mod_register_reset_callback(GBAModActivationCallback callback) {
    reset_callback = callback;
    return 1;
}
// A fresh installation has no external assets. The production plugin must
// never ask for one, even after enabling either or both features.
extern "C" const char* gba_mod_required_asset_path(const char*, const char*) {
    ++asset_requests;
    return nullptr;
}

#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #c); std::exit(1); } } while (0)

int main() {
    CHECK(activation_count == 2 && reset_callback);
    const char* ids[] = {"super-mario-world.restore-palettes", "super-mario-world.restore-instruments"};
    // Independent reference hashes: apply the original author IPS releases
    // to the synthetic i*37+19 cartridge, using a separate byte-level decoder.
    const char* expected[] = {
        "a7be4bcc78bc3c1703bd327f05acc59943b901fa",
        "3aeb17afc5fc98ce84ced85c95e39c20db722806",
        "583909bc82a7ec968a48ed2888df053ce576f12c",
        "bc58da0b147c517cf5b36296e40db059b95c9b39",
    };
    std::vector<std::uint8_t> original(0x400000);
    for (std::size_t i = 0; i < original.size(); ++i) original[i] = std::uint8_t(i * 37u + 19u);
    using Result = gbarecomp::ModRomDataResult;
    // Return to Off after each enabled combination to catch stale activation.
    for (unsigned mode : {0u, 1u, 0u, 2u, 0u, 3u, 0u}) {
        reset_callback();
        for (unsigned feature = 0; feature < 2; ++feature) {
            if (!(mode & (1u << feature))) continue;
            bool found = false;
            for (const auto& activation : activations) {
                if (activation.id && !std::strcmp(activation.id, ids[feature])) {
                    activation.callback(); found = true;
                }
            }
            CHECK(found);
        }
        auto image = original;
        const char* error = nullptr;
        CHECK(smw::prepare_restoration_rom_data(image.data(), image.size(), &error) ==
              (mode ? Result::Prepared : Result::Unchanged));
        CHECK(!error && asset_requests == 0);
        CHECK(gba::sha1(image.data(), image.size()).hex() == expected[mode]);
        if (mode) {
            const auto prepared = image;
            CHECK(smw::prepare_restoration_rom_data(image.data(), image.size(), &error) == Result::Prepared);
            CHECK(image == prepared && asset_requests == 0);
            std::vector<std::uint8_t> wrong_size(256, 0x5a), unchanged = wrong_size;
            CHECK(smw::prepare_restoration_rom_data(wrong_size.data(), wrong_size.size(), &error) == Result::Failed);
            CHECK(error && wrong_size == unchanged);
        }
    }
    std::puts("PASS: independent restoration toggles, exact reference data, reset/Off, no external assets and bounded failure");
}

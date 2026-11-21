#include "mod_runtime.h"
#include "../smw_restoration.h"
#include "../smw_restoration_patch.h"
#include "smw_restoration_payloads.h"
#include <algorithm>
#include <cstring>

namespace {
bool palette_enabled, instruments_enabled;
void reset() { palette_enabled = instruments_enabled = false; }
void palette() { palette_enabled = true; }
void instruments() { instruments_enabled = true; }

template<std::size_t Size>
bool load(const std::array<std::uint8_t, Size>& payload, std::size_t rom_size,
          std::vector<smw::restoration::Record>* plan, const char** error) {
    // The build verifies the original author payloads before embedding them.
    // Runtime activation never depends on an external patch or selected path.
    const std::vector<std::uint8_t> bytes(payload.begin(), payload.end());
    return smw::restoration::decode_ips(bytes, rom_size, plan, error);
}
}  // namespace

GBA_MOD_CONSTRUCTOR(smw_register_restoration_plugins) {
    (void)gba_mod_register_reset_callback(reset);
    (void)gba_mod_register_activation_plugin("super-mario-world.restore-palettes", palette);
    (void)gba_mod_register_activation_plugin("super-mario-world.restore-instruments", instruments);
}

gbarecomp::ModRomDataResult smw::prepare_restoration_rom_data(
        std::uint8_t* data, std::size_t size, const char** error) {
    using Result = gbarecomp::ModRomDataResult;
    if (!palette_enabled && !instruments_enabled) return Result::Unchanged;
    if (!data || size != 0x400000) {
        if (error) *error = "Restoration requires the supported USA cartridge";
        return Result::Failed;
    }
    std::vector<restoration::Record> plan;
    if (palette_enabled && !load(restoration::builtin::kPalettePatch, size, &plan, error))
        return Result::Failed;
    if (instruments_enabled &&
        (!load(restoration::builtin::kBongoPatch, size, &plan, error) ||
         !load(restoration::builtin::kHornPatch, size, &plan, error)))
        return Result::Failed;
    std::sort(plan.begin(), plan.end(), [](const auto& a, const auto& b) { return a.offset < b.offset; });
    for (std::size_t i = 1; i < plan.size(); ++i)
        if (plan[i - 1].offset + plan[i - 1].bytes.size() > plan[i].offset) {
            if (error) *error = "Selected restoration assets overlap";
            return Result::Failed;
        }
    // Validate every asset/record before changing the process-local image.
    // These exact payloads produce byte-identical generated instructions,
    // literal handling and configured copied code to the original cartridge.
    for (const auto& record : plan)
        std::memcpy(data + record.offset, record.bytes.data(), record.bytes.size());
    return Result::Prepared;
}

#include "mod_runtime.h"
#include "sha1.h"
#include "../smw_restoration.h"
#include "../smw_restoration_patch.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace {
constexpr const char* kPalette = "super-mario-world.enhancement.palette-restoration";
constexpr const char* kInstruments = "super-mario-world.enhancement.instrument-restoration";
bool palette_enabled, instruments_enabled;
void reset() { palette_enabled = instruments_enabled = false; }
void palette() { palette_enabled = true; }
void instruments() { instruments_enabled = true; }

bool load(const char* package, const char* asset, std::size_t size,
          const char* sha1, std::size_t rom_size,
          std::vector<smw::restoration::Record>* plan, const char** error) {
    auto fail = [&](const char* reason) { if (error) *error = reason; return false; };
    const char* path = gba_mod_required_asset_path(package, asset);
    if (!path) return fail("A selected restoration asset is not committed");
    std::ifstream file(std::filesystem::path(reinterpret_cast<const char8_t*>(path)),
                       std::ios::binary | std::ios::ate);
    if (!file || file.tellg() != std::streamoff(size))
        return fail("A restoration asset is missing or has changed size");
    std::vector<std::uint8_t> bytes(size);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()), bytes.size()) ||
        gba::sha1(bytes.data(), bytes.size()).hex() != sha1)
        return fail("A restoration asset has changed since selection");
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
    if (palette_enabled && !load(kPalette, "palette-patch", 5975,
            "984f5bd9c612e567a4b5412927b64ab65b9e2d48", size, &plan, error))
        return Result::Failed;
    if (instruments_enabled &&
        (!load(kInstruments, "bongo-patch", 4040,
             "b83352df126919706e3618ba4976471a32c5fc99", size, &plan, error) ||
         !load(kInstruments, "horn-patch", 15,
             "96480df7633dd760dd47efe08470867fcc209482", size, &plan, error)))
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

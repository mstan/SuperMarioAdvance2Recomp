#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace smw::restoration {
struct Record {
    std::size_t offset;
    std::vector<std::uint8_t> bytes;
};
// Bounded IPS records only; no resizing, cartridge headers or overlapping
// writes. Failure leaves the caller's plan unchanged.
bool decode_ips(const std::vector<std::uint8_t>& input, std::size_t rom_size,
                std::vector<Record>* output, const char** error);
}  // namespace smw::restoration

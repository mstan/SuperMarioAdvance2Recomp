#include "smw_restoration_patch.h"
#include <algorithm>
#include <cstring>

namespace smw::restoration {
bool decode_ips(const std::vector<std::uint8_t>& input, std::size_t rom_size,
                std::vector<Record>* output, const char** error) {
    auto fail = [&](const char* reason) { if (error) *error = reason; return false; };
    if (!output || input.size() < 8 || std::memcmp(input.data(), "PATCH", 5))
        return fail("Invalid restoration patch header");
    std::vector<Record> parsed;
    std::size_t at = 5;
    auto word = [&](std::size_t p) { return unsigned(input[p]) * 256u + input[p + 1]; };
    while (true) {
        if (input.size() - at < 3) return fail("Truncated restoration patch");
        if (!std::memcmp(input.data() + at, "EOF", 3)) {
            if (at + 3 != input.size()) return fail("Restoration patch cannot resize the cartridge");
            break;
        }
        if (input.size() - at < 5) return fail("Truncated restoration patch record");
        const std::size_t offset = (std::size_t(input[at]) << 16) |
            (std::size_t(input[at + 1]) << 8) | input[at + 2];
        std::size_t count = word(at + 3);
        at += 5;
        Record record{offset, {}};
        if (count) {
            if (count > input.size() - at) return fail("Truncated restoration patch data");
            record.bytes.assign(input.begin() + at, input.begin() + at + count);
            at += count;
        } else {
            if (input.size() - at < 3 || !(count = word(at)))
                return fail("Invalid restoration patch repeat");
            record.bytes.assign(count, input[at + 2]);
            at += 3;
        }
        if (offset < 0xC0 || offset > rom_size || count > rom_size - offset)
            return fail("Restoration patch touches a header or exceeds the cartridge");
        parsed.push_back(std::move(record));
    }
    std::sort(parsed.begin(), parsed.end(), [](const auto& a, const auto& b) {
        return a.offset < b.offset;
    });
    for (std::size_t i = 1; i < parsed.size(); ++i)
        if (parsed[i - 1].offset + parsed[i - 1].bytes.size() > parsed[i].offset)
            return fail("Restoration patch records overlap");
    output->insert(output->end(), parsed.begin(), parsed.end());
    return true;
}
}  // namespace smw::restoration

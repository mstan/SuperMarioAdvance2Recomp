#include "smw_restoration_patch.h"
#include <cstdio>
#include <cstdlib>

#define CHECK(c) do { if (!(c)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #c); std::exit(1); } } while (0)
using Bytes = std::vector<std::uint8_t>;
using Plan = std::vector<smw::restoration::Record>;
static Bytes header() { return {'P','A','T','C','H'}; }
static void record(Bytes& b, unsigned offset, Bytes data) {
    b.insert(b.end(), {std::uint8_t(offset >> 16), std::uint8_t(offset >> 8), std::uint8_t(offset),
                     std::uint8_t(data.size() >> 8), std::uint8_t(data.size())});
    b.insert(b.end(), data.begin(), data.end());
}
static void eof(Bytes& b) { b.insert(b.end(), {'E','O','F'}); }
static void rejected(Bytes b, std::size_t size = 0x200) {
    Plan plan{{0x1f0,{7,8,9}}}; const char* error = nullptr;
    CHECK(!smw::restoration::decode_ips(b, size, &plan, &error));
    CHECK(error && plan.size() == 1 && plan[0].offset == 0x1f0 && plan[0].bytes == Bytes({7,8,9}));
}
int main() {
    auto valid = header(); record(valid, 0x100, {1,2,3});
    valid.insert(valid.end(), {0,1,0x10,0,0,0,5,0xcc}); eof(valid);
    Plan plan; const char* error = nullptr;
    CHECK(smw::restoration::decode_ips(valid, 0x200, &plan, &error));
    CHECK(plan.size() == 2 && plan[0].offset == 0x100 && plan[0].bytes == Bytes({1,2,3}));
    CHECK(plan[1].offset == 0x110 && plan[1].bytes == Bytes(5,0xcc));
    auto bad = valid; bad.pop_back(); rejected(bad);
    bad = valid; bad.push_back(0); rejected(bad);
    bad = valid; bad[0] = 'X'; rejected(bad);
    bad = header(); record(bad, 0xbf, {1}); eof(bad); rejected(bad);
    bad = header(); record(bad, 0x1ff, {1,2}); eof(bad); rejected(bad);
    bad = header(); bad.insert(bad.end(), {0,1,0,0,0,0,0,7}); eof(bad); rejected(bad);
    bad = header(); record(bad, 0x100, {1,2}); record(bad, 0x101, {3,4}); eof(bad); rejected(bad);
    bad = header(); bad.insert(bad.end(), {0,1,0,0,5,1,2}); rejected(bad);
    std::puts("Restoration literal/RLE, bounds/header/truncation/overlap rejection and atomic plan PASS");
}

#pragma once
#include "runtime.h"
namespace smw {
gbarecomp::ModRomDataResult prepare_restoration_rom_data(
    std::uint8_t* data, std::size_t size, const char** error);
}

#pragma once

#include <cstdint>

namespace gbarecomp { struct ExtendedViewFrameInfo; }

namespace smw {

void update_extended_view(const gbarecomp::ExtendedViewFrameInfo* frame);

void install_extended_view(std::uint32_t extra_left,
                           std::uint32_t extra_right);

}  // namespace smw

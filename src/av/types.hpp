#pragma once

#include <cstdint>
#include <vector>

namespace glimpse::av {
struct VideoFrame {
    int width;
    int height;
    int linesize;
    std::vector<uint8_t> pixels;
};
} // namespace glimpse::av

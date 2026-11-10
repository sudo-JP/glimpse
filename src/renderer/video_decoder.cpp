#include "video_decoder.hpp"

namespace glimpse::renderer {
namespace {
    
} // namespace

std::expected<VideoDecoder, std::string> VideoDecoder::new_video_decoder(
    const std::string& filename
) {

    return VideoDecoder();
}
} // namespace glimpse::renderer

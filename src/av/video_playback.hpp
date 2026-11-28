#pragma once

#include "av/types.hpp"
#include "av/video_decoder.hpp"
#include "utils/queue.hpp"
#include <thread>

namespace glimpse::av {
class VideoPlayback {
public:
private:
av::VideoDecoder m_video_decoder;
utils::ConcurrentQueue<VideoFrame> m_video_frame_queue;
std::jthread m_jthread;
};
 
} // namespace glimpse::av

#pragma once

#include "renderer/types.hpp"
#include <expected>
#include <memory>
#include <string>
extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
}

namespace glimpse::renderer {

struct AVFormatContextDeleter {
    void operator()(AVFormatContext* ctx) const { if (ctx) avformat_close_input(&ctx); }
};

struct AVCodecContextDeleter {
    void operator()(AVCodecContext* ctx) const { if (ctx) avcodec_free_context(&ctx); }
};

struct SwsContextDeleter {
    void operator()(SwsContext* ctx) const { if (ctx) sws_freeContext(ctx); }
};


class VideoDecoder {
public:
    enum class NextFrameError {
        EndOfFile,
        ReadError,
    };

    static std::expected<VideoDecoder, std::string> new_video_decoder(
        const std::string& filename
    );

    std::expected<VideoFrame, NextFrameError> next_frame();

    /// Reset stream of video to start
    void reset();
private:
    VideoDecoder(
        std::unique_ptr<AVFormatContext, AVFormatContextDeleter> format_ctx,
        std::unique_ptr<AVCodecContext, AVCodecContextDeleter> codec,
        std::unique_ptr<SwsContext, SwsContextDeleter> sws_ctx
    );


    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> m_format_ctx;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> m_codec;
    std::unique_ptr<SwsContext, SwsContextDeleter> m_sws_ctx;
    int m_stream_idx = 0;
};
} // namespace glimpse::renderer

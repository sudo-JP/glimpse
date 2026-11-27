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

struct AVFrameDeleter {
    void operator()(AVFrame* frame) const { if (frame) av_frame_free(&frame); }
};

struct AVPacketDeleter {
    void operator()(AVPacket* packet) const { if (packet) av_packet_free(&packet); }
};


class VideoDecoder {
public:
    enum class NextFrameError {
        EndOfFile,
        ReadError,
        AllocationError,
    };

    static std::expected<VideoDecoder, std::string> new_video_decoder(
        const std::string& filename,
        std::string texture_name
    );

    std::expected<VideoFrame, NextFrameError> next_frame();

    /// Reset stream of video to start
    std::expected<void, NextFrameError> reset();

    const std::string get_texture_name() const;
private:
    struct AVContext {
        std::unique_ptr<AVFormatContext, AVFormatContextDeleter> format_ctx;
        std::unique_ptr<AVCodecContext, AVCodecContextDeleter> codec;
        std::unique_ptr<SwsContext, SwsContextDeleter> sws_ctx;
    };

    struct FrameDecoder {
        std::unique_ptr<AVFrame, AVFrameDeleter> av_frame;
        std::unique_ptr<AVFrame, AVFrameDeleter> rgb_frame;
        std::unique_ptr<AVPacket, AVPacketDeleter> packet;
    };

    VideoDecoder(
        AVContext ctx,        
        FrameDecoder frame_decoder,
        std::string texture_name,
        int stream_idx
    );


    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> m_format_ctx;
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> m_codec;
    std::unique_ptr<SwsContext, SwsContextDeleter> m_sws_ctx;

    std::unique_ptr<AVFrame, AVFrameDeleter> m_av_frame;
    std::unique_ptr<AVFrame, AVFrameDeleter> m_rgb_frame;
    std::unique_ptr<AVPacket, AVPacketDeleter> m_packet;

    bool m_flushing = false;
    int m_stream_idx;

    std::string m_texture_name;
};
} // namespace glimpse::renderer

#include "video_decoder.hpp"
#include "renderer/types.hpp"
#include <cstdint>
#include <libavutil/frame.h>
#include <utility>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <memory>

namespace glimpse::av {
namespace {
constexpr int texture_width = 3840;
constexpr int texture_height = 2160;
} // namespace

std::expected<VideoDecoder, std::string> VideoDecoder::new_video_decoder(
    const std::string& filename,
    std::string texture_name
) {
    AVFormatContext *raw_ctx = nullptr; 
    if (avformat_open_input(&raw_ctx, filename.c_str(), nullptr, nullptr) < 0) {
        return std::unexpected("could not open file");
    }
    auto format_ctx = std::unique_ptr<AVFormatContext, AVFormatContextDeleter>(raw_ctx);

    if (avformat_find_stream_info(format_ctx.get(), nullptr) < 0) {
        return std::unexpected("could not find stream info");
    }

    int video_stream_idx = -1;
    const AVCodec *raw_codec = nullptr;
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        // Maybe handle more types of decoding later on
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            raw_codec = avcodec_find_decoder(format_ctx->streams[i]->codecpar->codec_id);
        }
    }
    if (video_stream_idx == -1 || !raw_codec) {
        return std::unexpected("could not find valid stream for video");
    }

    AVCodecContext *raw_codec_ctx = avcodec_alloc_context3(raw_codec);
    if (!raw_codec_ctx) {
        return std::unexpected("could not allocate codec context");
    }
    auto codec_ctx = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>(raw_codec_ctx);

    if (avcodec_parameters_to_context(codec_ctx.get(), format_ctx->streams[video_stream_idx]->codecpar) < 0) {
        return std::unexpected("failed to copy codec parameters to context");
    }
    if (avcodec_open2(codec_ctx.get(), raw_codec, nullptr) < 0) {
        return std::unexpected("failed to open codec");
    }

    int width = codec_ctx->width;
    int height = codec_ctx->height;

    
    SwsContext *raw_sws_ctx = sws_getContext(
        width, height, codec_ctx->pix_fmt,
        texture_width, texture_height, AV_PIX_FMT_RGBA,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!raw_sws_ctx) return std::unexpected("could not create sws context");
    auto sws_ctx = std::unique_ptr<SwsContext, SwsContextDeleter>(raw_sws_ctx);


    AVContext ctx{
        std::move(format_ctx),
        std::move(codec_ctx),
        std::move(sws_ctx)
    };

    AVFrame *raw_av_frame = av_frame_alloc();
    if (!raw_av_frame) return std::unexpected("failed to allocate av frame");
    auto av_frame = std::unique_ptr<AVFrame, AVFrameDeleter>(raw_av_frame);

    AVFrame *raw_rgb_frame = av_frame_alloc();
    if (!raw_rgb_frame) return std::unexpected("failed to allocate rgb frame");
    auto rgb_frame = std::unique_ptr<AVFrame, AVFrameDeleter>(raw_rgb_frame);

    AVPacket *raw_packet = av_packet_alloc();
    if (!raw_packet) return std::unexpected("fail to allocate packet");
    auto packet = std::unique_ptr<AVPacket, AVPacketDeleter>(raw_packet);

    FrameDecoder frame_decoder{
        std::move(av_frame),
        std::move(rgb_frame),
        std::move(packet)
    };

    return VideoDecoder(
        std::move(ctx),
        std::move(frame_decoder),
        std::move(texture_name),
        video_stream_idx
    );
}

std::expected<VideoFrame, VideoDecoder::NextFrameError> VideoDecoder::next_frame() {
    constexpr int width = texture_width;
    constexpr int height = texture_height;

    VideoFrame frame;
    frame.width = width;
    frame.height = height;
    frame.pixels.resize(width * height * 4);

    uint8_t* dest_data[4] = { frame.pixels.data(), nullptr, nullptr, nullptr };
    int dest_linesize[4] = { width * 4, 0, 0, 0 };

    while (true) {
        int ret = avcodec_receive_frame(m_codec.get(), m_av_frame.get());
        if (ret == 0) {
            sws_scale(
                m_sws_ctx.get(),
                m_av_frame->data,
                m_av_frame->linesize,
                0,
                m_codec->height,
                dest_data,
                dest_linesize
            );
            av_frame_unref(m_av_frame.get());
            return frame;
        }
        if (ret == AVERROR_EOF) {
            return std::unexpected(VideoDecoder::NextFrameError::EndOfFile);
        }
        if (ret != AVERROR(EAGAIN)) {
            return std::unexpected(VideoDecoder::NextFrameError::ReadError);
        }

        if (m_flushing) return std::unexpected(VideoDecoder::NextFrameError::EndOfFile);

        ret = av_read_frame(m_format_ctx.get(), m_packet.get());
        if (ret == AVERROR_EOF) {
            m_flushing = true;
            avcodec_send_packet(m_codec.get(), nullptr);
            continue;
        }
        if (ret < 0) {
            return std::unexpected(VideoDecoder::NextFrameError::ReadError);
        }

        if (m_packet->stream_index != m_stream_idx) {
            av_packet_unref(m_packet.get());
            continue;
        }

        ret = avcodec_send_packet(m_codec.get(), m_packet.get());
        av_packet_unref(m_packet.get());

        if (ret < 0) {
            return std::unexpected(VideoDecoder::NextFrameError::ReadError);
        }
    }
}

std::expected<void, VideoDecoder::NextFrameError> VideoDecoder::reset() {
    int ret = av_seek_frame(m_format_ctx.get(), m_stream_idx, 0, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) return std::unexpected(VideoDecoder::NextFrameError::ReadError);
    
    avcodec_flush_buffers(m_codec.get());

    m_flushing = false;
    return {};
}

const std::string VideoDecoder::get_texture_name() const {
    return m_texture_name;
}

VideoDecoder::VideoDecoder(
    AVContext ctx,        
    FrameDecoder frame_decoder,
    std::string texture_name,
    int stream_idx
) : m_format_ctx(std::move(ctx.format_ctx)),
m_codec(std::move(ctx.codec)),
m_sws_ctx(std::move(ctx.sws_ctx)), 
m_av_frame(std::move(frame_decoder.av_frame)),
m_rgb_frame(std::move(frame_decoder.rgb_frame)),
m_packet(std::move(frame_decoder.packet)),
m_stream_idx(stream_idx),
m_texture_name(std::move(texture_name))
{}
} // namespace glimpse::av

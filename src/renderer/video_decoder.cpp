#include "video_decoder.hpp"
#include "renderer/types.hpp"
#include <cstdint>
#include <utility>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <memory>

namespace glimpse::renderer {
namespace {
    struct AVFrameDeleter {
        void operator()(AVFrame* frame) const { if (frame) av_frame_free(&frame); }
    };

    struct AVPacketDeleter {
        void operator()(AVPacket* packet) const { if (packet) av_packet_free(&packet); }
    };
} // namespace

std::expected<VideoDecoder, std::string> VideoDecoder::new_video_decoder(
    const std::string& filename
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
        width, height, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!raw_sws_ctx) return std::unexpected("could not create sws context");
    auto sws_ctx = std::unique_ptr<SwsContext, SwsContextDeleter>(raw_sws_ctx);


    return VideoDecoder(
        std::move(format_ctx),
        std::move(codec_ctx),
        std::move(sws_ctx),
        video_stream_idx
    );
}

std::expected<VideoFrame, VideoDecoder::NextFrameError> VideoDecoder::next_frame() {
    // Allocate rgb buffer
    //uint8_t *rgb_data[4];
    //int rgb_linesize[4];
    //av_image_alloc(rgb_data, rgb_linesize, width, height, AV_PIX_FMT_RGB24, 1);
    AVFrame *raw_av_frame = av_frame_alloc();
    if (!raw_av_frame) return std::unexpected(VideoDecoder::NextFrameError::AllocationError);
    auto av_frame = std::unique_ptr<AVFrame, AVFrameDeleter>(raw_av_frame);

    AVPacket *raw_packet = av_packet_alloc();
    if (!raw_packet) return std::unexpected(VideoDecoder::NextFrameError::AllocationError);
    auto packet = std::unique_ptr<AVPacket, AVPacketDeleter>(raw_packet);


    // Setup frame to be returned
    VideoFrame frame;
    int width = m_codec->width;
    int height = m_codec->height;

    frame.width = width;
    frame.height = height;

    int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    frame.pixels.resize(num_bytes);

    av_image_fill_arrays(av_frame->data, av_frame->linesize, frame.pixels.data(), AV_PIX_FMT_RGB24, width, height, 1);
    while (true) {
        int ret = av_read_frame(m_format_ctx.get(), raw_packet);
        if (ret == AVERROR_EOF) return std::unexpected(VideoDecoder::NextFrameError::EndOfFile);
        if (raw_packet->stream_index != m_stream_idx) continue;

        // Send packet
        ret = avcodec_send_packet(m_codec.get(), packet.get());
        if (ret != 0) continue;

        while (avcodec_receive_frame(m_codec.get(), av_frame.get())) {
            //sws_scale(m_sws_ctx.get(), av_frame->data, av_frame->linesize, 0, height,
        }
    }

    return frame;
}

void VideoDecoder::reset() {
    
}

VideoDecoder::VideoDecoder(
    std::unique_ptr<AVFormatContext, AVFormatContextDeleter> format_ctx,
    std::unique_ptr<AVCodecContext, AVCodecContextDeleter> codec,
    std::unique_ptr<SwsContext, SwsContextDeleter> sws_ctx,
    int stream_idx
) : m_format_ctx(std::move(format_ctx)),
m_codec(std::move(codec)),
m_sws_ctx(std::move(sws_ctx)), m_stream_idx(stream_idx) {}
} // namespace glimpse::renderer

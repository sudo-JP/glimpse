#include "video_decoder.hpp"
#include <cstdint>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
#include <memory>

namespace glimpse::renderer {
namespace {
    
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

    // Allocate rgb buffer

    uint8_t *rgb_data[4];
    int rgb_linesize[4];
    av_image_alloc(rgb_data, rgb_linesize, width, height, AV_PIX_FMT_RGB24, 1);

    AVPacket *packet = av_packet_alloc();
    if (!packet) return std::unexpected("could not create packet");

    AVFrame *frame = av_frame_alloc();
    if (!frame) return std::unexpected("could not create frame");
    int frame_count = 0;

    while (av_read_frame(format_ctx.get(), packet)) {
        if (packet->stream_index == video_stream_idx) {

        }
    }



    return VideoDecoder();
}

VideoDecoder::~VideoDecoder() {

}

VideoDecoder::VideoDecoder() {}
} // namespace glimpse::renderer

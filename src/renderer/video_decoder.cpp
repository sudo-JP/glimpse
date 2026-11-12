#include "video_decoder.hpp"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
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
    const AVCodec *codec = nullptr;
    for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
        if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            codec = avcodec_find_decoder(format_ctx->streams[i]->codecpar->codec_id);
        }
    }
    if (video_stream_idx == -1 || !codec) {
        return std::unexpected("could not find valid stream for video");
    }

    AVCodecContext *raw_codec_ctx = avcodec_alloc_context3(codec);
    if (!raw_codec_ctx) {
        return std::unexpected("could not allocate codec context");
    }
    auto codec_ctx = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>(raw_codec_ctx);

    if (avcodec_parameters_to_context(codec_ctx.get(), format_ctx->streams[video_stream_idx]->codecpar) < 0) {
        return std::unexpected("failed to copy codec parameters to context");
    }


    return VideoDecoder();
}

VideoDecoder::~VideoDecoder() {

}

VideoDecoder::VideoDecoder() {}
} // namespace glimpse::renderer

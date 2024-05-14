#include <emscripten.h>
#include <emscripten/bind.h>
#include <inttypes.h>

#include <string>
#include <vector>

using namespace emscripten;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>
#include <libavutil/timestamp.h>
}

struct VideoData {
    std::vector<uint8_t> data;

    VideoData(int size) : data(size) {}
    size_t getSize() const { return data.size(); }
};

VideoData cut_video(double start_seconds, double end_seconds, const std::string& inputFileName) {
    AVFormatContext *inputfile = nullptr, *outputfile = nullptr;
    AVPacket pkt;
    av_log_set_level(AV_LOG_INFO);

    if (avformat_open_input(&inputfile, inputFileName.c_str(), nullptr, nullptr) < 0) {
        EM_ASM({ console.error('Could not open source file'); });
        return VideoData(0);
    }

    if (avformat_find_stream_info(inputfile, nullptr) < 0) {
        avformat_close_input(&inputfile);
        EM_ASM({ console.error('Could not find stream info'); });
        return VideoData(0);
    }

    avformat_alloc_output_context2(&outputfile, nullptr, "mp4", nullptr);
    // Additional setup for output streams, similar to your existing C code...

    // Assume we have a buffer to write to
    AVIOContext *avio_ctx = nullptr;
    uint8_t *avio_ctx_buffer = static_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 1, nullptr, nullptr, nullptr, nullptr);

    outputfile->pb = avio_ctx;
    avformat_write_header(outputfile, nullptr);

    // Read frames, process them and write them to output format as shown in your existing C code...

    av_write_trailer(outputfile);

    // Copy the data to the buffer that will be returned
    VideoData videoData(avio_ctx->pos);
    std::copy(avio_ctx_buffer, avio_ctx_buffer + avio_ctx->pos, videoData.data.begin());

    av_free(avio_ctx_buffer);
    avio_context_free(&avio_ctx);
    avformat_free_context(outputfile);
    avformat_close_input(&inputfile);

    return videoData;
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("cut_video", &cut_video);
    class_<VideoData>("VideoData")
        .constructor<int>()
        .function("getSize", &VideoData::getSize)
        .property("data", &VideoData::data);
}

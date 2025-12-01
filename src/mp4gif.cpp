// mp4gif.cpp : Convert MP4 video to GIF using FFmpeg libraries
// Usage: mp4gif input.mp4 output.gif
// Optional third argument for custom pre-filters cropping: mp4gif input.mp4 output.gif "crop=w:h:x:y"

// Similar example using ffmpeg tools: ffmpeg -i input.mp4  -vf "fps=15,scale=800:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -loop 0 output.gif


#include <iostream>

// Include FFmpeg headers
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

// ... (cleanup() function and global declarations remain the same) ...
AVFormatContext* inFmtCtx = nullptr;
AVFormatContext* outFmtCtx = nullptr;
AVCodecContext* inCodecCtx = nullptr;
AVCodecContext* outCodecCtx = nullptr;
AVFilterGraph* filter_graph = nullptr;

void cleanup() {
    if (filter_graph) avfilter_graph_free(&filter_graph);
    if (inCodecCtx) avcodec_free_context(&inCodecCtx);
    if (outCodecCtx) avcodec_free_context(&outCodecCtx);
    if (inFmtCtx) avformat_close_input(&inFmtCtx);
    if (outFmtCtx && outFmtCtx->pb) avio_closep(&outFmtCtx->pb);
    if (outFmtCtx) avformat_free_context(outFmtCtx);
}


int main(int argc, char** argv) {
    if (argc < 3) {
		std::cerr << "mp4gif v1.0.0 Converts MP4 videos to animated GIFs using FFmpeg libraries." << std::endl;
        std::cerr << "Usage: mp4gif <input.mp4> <output.gif> [\"pre_filters\"]" << std::endl;
        std::cerr << "Example (default): mp4gif input.mp4 output.gif" << std::endl;
        std::cerr << "Example (custom):  mp4gif input.mp4 output.gif \"crop=400:400:500:400,fps=20,scale=800:-1:flags=lanczos\"" << std::endl;
        return 1;
    }
    const char* in_filename = argv[1];
    const char* out_filename = argv[2];

    // Handle optional filter string argument ---
    std::string pre_filters;
    if (argc == 4) {
        pre_filters = argv[3];
        std::cout << "Using custom pre-filters: " << pre_filters << std::endl;
    } else {
        pre_filters = "fps=15";
        std::cout << "Using default pre-filters: " << pre_filters << std::endl;
    }

    // Build the final, full filter description string
    std::string filter_descr_str = pre_filters + ",split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse";
    const char* filter_descr = filter_descr_str.c_str();

    // --- 1. Open Input & Find Video Stream  ---
    if (avformat_open_input(&inFmtCtx, in_filename, nullptr, nullptr) < 0) { std::cerr << "Error: Could not open input file.\n"; return 1; }
    if (avformat_find_stream_info(inFmtCtx, nullptr) < 0) { std::cerr << "Error: Could not find stream info.\n"; cleanup(); return 1; }

    int video_stream_index = av_find_best_stream(inFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream* inStream = inFmtCtx->streams[video_stream_index];
    const AVCodec* inCodec = avcodec_find_decoder(inStream->codecpar->codec_id);
    inCodecCtx = avcodec_alloc_context3(inCodec);
    avcodec_parameters_to_context(inCodecCtx, inStream->codecpar);
    avcodec_open2(inCodecCtx, inCodec, nullptr);

    // --- 2. Setup Complex Filtergraph (now uses the dynamic filter_descr) ---
    std::cout << "Configuring filtergraph..." << std::endl;
    filter_graph = avfilter_graph_alloc();
    AVFilterContext *buffersrc_ctx, *buffersink_ctx;
    
    char args[512];
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d", inCodecCtx->width, inCodecCtx->height, inCodecCtx->pix_fmt, inStream->time_base.num, inStream->time_base.den);
    avfilter_graph_create_filter(&buffersrc_ctx, avfilter_get_by_name("buffer"), "in", args, NULL, filter_graph);
    
    avfilter_graph_create_filter(&buffersink_ctx, avfilter_get_by_name("buffersink"), "out", NULL, NULL, filter_graph);
    AVPixelFormat pix_fmts[] = { AV_PIX_FMT_PAL8, AV_PIX_FMT_NONE };
    av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    
    AVFilterInOut *outputs = avfilter_inout_alloc();
    outputs->name = av_strdup("in"); outputs->filter_ctx = buffersrc_ctx; outputs->pad_idx = 0; outputs->next = NULL;
    AVFilterInOut *inputs = avfilter_inout_alloc();
    inputs->name = av_strdup("out"); inputs->filter_ctx = buffersink_ctx; inputs->pad_idx = 0; inputs->next = NULL;
    
    if (avfilter_graph_parse_ptr(filter_graph, filter_descr, &inputs, &outputs, NULL) < 0) { std::cerr << "Error parsing filtergraph\n"; cleanup(); return 1; }
    if (avfilter_graph_config(filter_graph, NULL) < 0) { std::cerr << "Error configuring filtergraph\n"; cleanup(); return 1; }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    // --- The rest of the code (Setup Output, Processing Loop, Cleanup) ---
    
    avformat_alloc_output_context2(&outFmtCtx, nullptr, "gif", out_filename);
    const AVCodec* outCodec = avcodec_find_encoder(AV_CODEC_ID_GIF);
    AVStream* outStream = avformat_new_stream(outFmtCtx, outCodec);
    outCodecCtx = avcodec_alloc_context3(outCodec);
    
    outCodecCtx->width = av_buffersink_get_w(buffersink_ctx);
    outCodecCtx->height = av_buffersink_get_h(buffersink_ctx);
    outCodecCtx->pix_fmt = AV_PIX_FMT_PAL8;
    outCodecCtx->time_base = av_buffersink_get_time_base(buffersink_ctx);
    
    avcodec_open2(outCodecCtx, outCodec, nullptr);
    avcodec_parameters_from_context(outStream->codecpar, outCodecCtx);
    avio_open(&outFmtCtx->pb, out_filename, AVIO_FLAG_WRITE);
    avformat_write_header(outFmtCtx, nullptr);

    std::cout << "Processing video frames..." << std::endl;
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    AVFrame* filt_frame = av_frame_alloc();

    while (av_read_frame(inFmtCtx, packet) >= 0) {
        if (packet->stream_index == video_stream_index) {
            avcodec_send_packet(inCodecCtx, packet);
            while (avcodec_receive_frame(inCodecCtx, frame) == 0) {
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) break;
                while (av_buffersink_get_frame(buffersink_ctx, filt_frame) == 0) {
                    AVPacket* out_pkt = av_packet_alloc();
                    avcodec_send_frame(outCodecCtx, filt_frame);
                    while (avcodec_receive_packet(outCodecCtx, out_pkt) == 0) {
                        av_packet_rescale_ts(out_pkt, outCodecCtx->time_base, outStream->time_base);
                        av_interleaved_write_frame(outFmtCtx, out_pkt);
                        av_packet_unref(out_pkt);
                    }
                    av_frame_unref(filt_frame);
                }
            }
        }
        av_packet_unref(packet);
    }
    
	// Flush remaining frames in filter and encoder
    av_buffersrc_add_frame(buffersrc_ctx, NULL);
    while (av_buffersink_get_frame(buffersink_ctx, filt_frame) == 0) {
        AVPacket* out_pkt = av_packet_alloc();
        avcodec_send_frame(outCodecCtx, filt_frame);
        while(avcodec_receive_packet(outCodecCtx, out_pkt) == 0) {
            av_packet_rescale_ts(out_pkt, outCodecCtx->time_base, outStream->time_base);
            av_interleaved_write_frame(outFmtCtx, out_pkt);
            av_packet_unref(out_pkt);
        }
        av_frame_unref(filt_frame);
    }
    
	// Flush encoder
    avcodec_send_frame(outCodecCtx, NULL);
    while (avcodec_receive_packet(outCodecCtx, packet) == 0) {
        av_packet_rescale_ts(packet, outCodecCtx->time_base, outStream->time_base);
        av_interleaved_write_frame(outFmtCtx, packet);
        av_packet_unref(packet);
    }
    
    av_write_trailer(outFmtCtx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    av_packet_free(&packet);
    cleanup();
    
    std::cout << "Conversion complete." << std::endl;
    return 0;
}
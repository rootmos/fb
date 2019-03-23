#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

static struct {
    AVCodecContext* cc;
    AVFormatContext* fc;
    AVFrame* frame;
    AVPacket* pkt;
    AVStream* st;
} enc_state;

static void dump_pkt()
{
    trace("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d",
         av_ts2str(enc_state.pkt->pts), av_ts2timestr(enc_state.pkt->pts, &enc_state.cc->time_base),
         av_ts2str(enc_state.pkt->dts), av_ts2timestr(enc_state.pkt->dts, &enc_state.cc->time_base),
         av_ts2str(enc_state.pkt->duration), av_ts2timestr(enc_state.pkt->duration, &enc_state.cc->time_base),
         enc_state.pkt->stream_index);
}

color_t* enc_initialize(size_t width, size_t height, const char* fn)
{
    // codec
    const char* codec_name = "libx264rgb";
    const AVCodec* codec = avcodec_find_encoder_by_name(codec_name);
    if(!codec) { failwith("unable to find codec: %s", codec_name); }

    // format
    int r = avformat_alloc_output_context2(&enc_state.fc, NULL, NULL, fn);
    if(r < 0) { failwith("unable to allocate format context"); }

    enc_state.cc = avcodec_alloc_context3(codec);
    if(!enc_state.cc) { failwith("unable to create context"); }

    if (enc_state.fc->oformat->flags & AVFMT_GLOBALHEADER) {
        enc_state.cc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    enc_state.cc->width = width;
    enc_state.cc->height = height;
    enc_state.cc->time_base = (AVRational){1, 24};
    enc_state.cc->framerate = (AVRational){24, 1};
    enc_state.cc->pix_fmt = AV_PIX_FMT_RGB24;

    r = av_opt_set(enc_state.cc->priv_data, "preset", "fast", 0);
    if(r < 0) { failwith("unable to set preset"); }

    r = av_opt_set(enc_state.cc->priv_data, "profile", "high444", 0);
    if(r < 0) { failwith("unable to set profile"); }

    r = avcodec_open2(enc_state.cc, codec, NULL);
    if(r < 0) { failwith("unable to open codec"); }

    // frame
    enc_state.frame = av_frame_alloc();
    if(!enc_state.frame) { failwith("unable to allocate frame"); }
    enc_state.frame->format = enc_state.cc->pix_fmt;
    enc_state.frame->width = width;
    enc_state.frame->height = height;

    r = av_frame_get_buffer(enc_state.frame, 0);
    if(r < 0) { failwith("av_frame_get_buffer failed"); }

    r = av_frame_make_writable(enc_state.frame);
    if(r < 0) { failwith("av_frame_make_writable failed"); }

    enc_state.pkt = av_packet_alloc();
    if(!enc_state.pkt) { failwith("unable to allocate packet"); }

    enc_state.st = avformat_new_stream(enc_state.fc, NULL);
    if(!enc_state.st) { failwith("unable to create new stream"); }
    enc_state.st->id = enc_state.fc->nb_streams-1;
    enc_state.st->time_base = enc_state.cc->time_base;

    r = avcodec_parameters_from_context(enc_state.st->codecpar, enc_state.cc);
    if(r < 0) { failwith("unable to copy codec params"); }

    av_dump_format(enc_state.fc, 0, fn, 1);

    r = avio_open(&enc_state.fc->pb, fn, AVIO_FLAG_WRITE);
    if(r < 0) { failwith("unable to open output file"); }

    r = avformat_write_header(enc_state.fc, NULL);
    if(r < 0) { failwith("unable to write header: %s", av_err2str(r)); }

    return (color_t*)enc_state.frame->data[0];
}

static void send_frame(AVFrame* frame)
{
    if(frame) trace("sending frame pts=%"PRId64, frame->pts);
    int r = avcodec_send_frame(enc_state.cc, frame);
    if(r < 0) { failwith("unable to send frame"); }

    while(r >= 0) {
        r = avcodec_receive_packet(enc_state.cc, enc_state.pkt);
        if(r == AVERROR(EAGAIN) || r == AVERROR_EOF) {
            break;
        }

        if(r < 0) {
            failwith("unable to receive packet: %s", av_err2str(r));
        }

        av_packet_rescale_ts(enc_state.pkt, enc_state.cc->time_base, enc_state.st->time_base);
        dump_pkt();

        r = av_write_frame(enc_state.fc, enc_state.pkt);
        if(r < 0) { failwith("av_write_frame failed"); }

        av_packet_unref(enc_state.pkt);
    }
}

void enc(size_t i)
{
    enc_state.frame->pts = i;
    send_frame(enc_state.frame);
}

void enc_finalize(int out)
{
    send_frame(NULL);

    int r = av_write_trailer(enc_state.fc);
    if(r < 0) { failwith("unable to write trailer"); }

    avcodec_free_context(&enc_state.cc);
    av_frame_free(&enc_state.frame);
    av_packet_free(&enc_state.pkt);
    avio_closep(&enc_state.fc->pb);
    avformat_free_context(enc_state.fc);
}

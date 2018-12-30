#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <sys/ioctl.h> // For FIONREAD
#include <termios.h>
#include <stdbool.h>


#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};

static void logger(const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}


static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    
    logger("%s:  stream_index:%d  pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s flags:%d\n",
           tag,
           pkt->stream_index,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->flags);
}

int kbhit(void)
{
    static bool initflag = false;
    static const int STDIN = 0;
    
    if (!initflag) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initflag = true;
    }
    
    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
    return nbbytes;
}


bool open_video_encoder()
{
    AVCodec *codec      = NULL;
    AVCodecContext *vc  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        logger("Unable to find libx264");
        return false;
    }
    vc = avcodec_alloc_context3(codec);
    
    ret = avcodec_open2(vc, codec, NULL);
    
    return true;
}

bool open_audio_encoder()
{
    AVCodec *codec      = NULL;
    AVCodecContext *vc  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("aac");
    if (!codec) {
        logger("Unable to find aac");
        return false;
    }
    vc = avcodec_alloc_context3(codec);
    
    ret = avcodec_open2(vc, codec, NULL);
    
    return true;
}



int process_in(AVFormatContext *pFC, AVFrame *frame, AVPacket *pkt)
{
    int ret = 0;
    
    av_init_packet(pkt);
    // loop until a new frame has been decoded, or EAGAIN
    while (1) {
        AVStream *ist = NULL;
        AVCodecContext *decoder = NULL;
        ret = av_read_frame(pFC, pkt);
        /*
        if (ret == AVERROR_EOF) {
            break;
        }
        else
        {
            if (ret < 0) {
                dec_err("Unable to read input\n");
            }
        }
        ist = pFC->streams[pkt->stream_index];
        if (ist->index == ictx->vi && ictx->vc) {
            decoder = ictx->vc;
        }
        else {
            if (ist->index == ictx->ai && ictx->ac) {
                decoder = ictx->ac;
            }
        }
        else {
            dec_err("Could not find decoder for stream\n");
        }
        
        ret = avcodec_send_packet(decoder, pkt);
        if (ret < 0) dec_err("Error sending packet to decoder\n");
        ret = avcodec_receive_frame(decoder, frame);
        if (ret == AVERROR(EAGAIN)) {
            av_packet_unref(pkt);
            continue;
        }
        else if (ret < 0) dec_err("Error receiving frame from decoder\n");
        break;*/
    }
    
    return ret;
    
#undef dec_err
}

#define MAX_INPUTS 10

struct Context {
    char *pSourceFileName;
    AVFormatContext *ifmt_ctx;
    AVCodec* input_codecs[MAX_INPUTS];
    AVCodecContext* input_codec_contexts[MAX_INPUTS];
};


void initContext(struct Context *pContext)
{
    pContext->ifmt_ctx=NULL;
    for (int i=0;i<MAX_INPUTS;i++) {
        pContext->input_codecs[i]=NULL;
        pContext->input_codec_contexts[i]=NULL;
    }
}

bool open_input(struct Context * pContext)
{
    int ret = avformat_open_input(&pContext->ifmt_ctx, pContext->pSourceFileName, NULL, NULL);
    
    if (ret < 0) {
        char buff[256];
        av_strerror(ret, buff, 256);
        logger("len: Unable to open input %s %s(%d)\n",pContext->pSourceFileName,buff,ret);
        return false;
        
    }
    ret = avformat_find_stream_info(pContext->ifmt_ctx, NULL);
    if (ret < 0) {
        logger("segmenter: Unable to find any input streams\n");
    }
    
    
    for (int i = 0; i < pContext->ifmt_ctx->nb_streams; i++) {
        AVStream *stream = pContext->ifmt_ctx->streams[i];
        AVCodec *dec = pContext->input_codecs[i]=avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec) {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
            return AVERROR_DECODER_NOT_FOUND;
        }
        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                   "for stream #%u\n", i);
            return ret;
        }
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
            || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
            codec_ctx->framerate = av_guess_frame_rate(pContext->ifmt_ctx, stream, NULL);
            /* Open decoder */
            ret = avcodec_open2(codec_ctx, dec, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
        pContext->input_codec_contexts[i] = codec_ctx;
    }
    return true;
}

void print_av_frame(AVFrame *pFrame,AVRational *time_base) {
    
    const char *pict_type;
    switch (pFrame->pict_type)
    {
        case AV_PICTURE_TYPE_I: pict_type="I"; break;     ///< Intra
        case AV_PICTURE_TYPE_P: pict_type="P"; break;      ///< Predicted
        case AV_PICTURE_TYPE_B: pict_type="B"; break;      ///< Bi-dir predicted
        case AV_PICTURE_TYPE_S: pict_type="S"; break;      ///< S(GMC)-VOP MPEG-4
        case AV_PICTURE_TYPE_SI: pict_type="SI"; break;     ///< Switching Intra
        case AV_PICTURE_TYPE_SP: pict_type="SP"; break;     ///< Switching Predicted
        case AV_PICTURE_TYPE_BI: pict_type="BI"; break;     ///< BI type
        default: pict_type="";
    }
    
    if (pFrame->width>0) {
        logger("decoded video: pts=%s (%s),frame type=%s;width=%d;height=%d",av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, time_base),pict_type,pFrame->width,pFrame->height);
    } else {
        logger("decoded audio: pts=%s (%s);channels=%d;sample rate=%d; length=%d; format=%d ",av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, time_base),pFrame->channels,pFrame->sample_rate,pFrame->nb_samples,pFrame->format);
    }
}





void OnInputFrame(struct Context *ctx,AVCodecContext* pDecoderContext,const AVFrame *pFrame)
{
    print_av_frame(pFrame,&pDecoderContext->time_base);
    
    if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
        printf("saving frame %3d\n", pDecoderContext->frame_number);
    } else {
        
    }
    
}

void processPacket(struct Context *ctx,const AVPacket* pkt) {

    int ret;
    
    int stream_index = pkt->stream_index;
    
    av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",stream_index);
    
    AVCodecContext* pDecoderContext=ctx->input_codec_contexts[stream_index];

    ret = avcodec_send_packet(pDecoderContext, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        AVFrame *pFrame = av_frame_alloc();
        int got_frame;
        
        ret = avcodec_receive_frame(pDecoderContext, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&pFrame);
            return;
        }
        else if (ret < 0)
        {
            logger("Error during decoding\n");
            return;
        }
        
        OnInputFrame(ctx,pDecoderContext,pFrame);
        
        av_frame_free(&pFrame);
    }
    
}


void process(struct Context *ctx)
{
    AVPacket packet;
    int ret;
    unsigned int stream_index;


    enum AVMediaType type;

    AVFormatContext* ifmt_ctx=ctx->ifmt_ctx;
    av_init_packet(&packet);

    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        
        processPacket(ctx,&packet);
    
    }
}

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);

    
    
    //logger("Version: %s\n", VERSION);

    avformat_network_init();
    
    struct Context ctx;
    initContext(&ctx);
    ctx.pSourceFileName="/Users/guyjacubovski/Sample_video/קישון - תעלת בלאומילך.avi";
    open_input(&ctx);


    process(&ctx);
 
    return 0;
}


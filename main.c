#define __STDC_CONSTANT_MACROS


#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
        
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include "logger.h"

#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};

#include "TranscodePipeline.h"
#include "listener.h"


void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if (level<AV_LOG_DEBUG)
        return;
    logger2("FFMPEG",level,fmt,vargs);
}

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);

  //  av_log_set_callback(ffmpeg_log_callback);
    
    char* pSourceFileName="/Users/guyjacubovski/Sample_video/קישון - תעלת בלאומילך.avi";

    AVFormatContext *ifmt_ctx;
    int ret = avformat_open_input(&ifmt_ctx, pSourceFileName, NULL, NULL);
    
    if (ret < 0) {
        char buff[256];
        av_strerror(ret, buff, 256);
        logger(AV_LOG_DEBUG,"Unable to open input %s %s(%x)",pSourceFileName,buff,ret);
        return ret;
        
    }
    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0) {
        logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"segmenter: Unable to find any input streams");
    }

    //logger("Version: %s\n", VERSION);

    avformat_network_init();
    
    struct TranscodeContext ctx;
    

    init_transcoding_context(&ctx,ifmt_ctx->streams[0]);


    struct TranscodeOutput output32;
    init_Transcode_output(&output32);
    
    output32.name="32";
    output32.codec_type=AVMEDIA_TYPE_VIDEO;
    output32.passthrough=true;
    
    add_output(&ctx,&output32);
    
    
    struct TranscodeOutput output33;
    init_Transcode_output(&output33);
    
    output33.name="33";
    output33.codec_type=AVMEDIA_TYPE_VIDEO;
    output33.passthrough=false;
    output33.width=352;
    output33.height=240;
    output33.fps=30;
    output33.vid_bitrate=500;
    
    add_output(&ctx,&output33);
    
    
    struct TranscodeOutput output34;
    init_Transcode_output(&output34);
    
    output34.name="34";
    output34.codec_type=AVMEDIA_TYPE_VIDEO;
    output34.passthrough=false;
    output34.width=352;
    output34.height=240;
    output34.fps=30;
    output34.vid_bitrate=200;
    
    add_output(&ctx,&output34);


    //startService(&ctx,9999);
    AVPacket packet;
    av_init_packet(&packet);
    
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        
        if (0!=packet.stream_index) {
            continue;
        }
        
        AVStream *in_stream=ifmt_ctx->streams[packet.stream_index];
        
        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, standard_timebase, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, standard_timebase, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, standard_timebase);
        
        convert_packet(&ctx,in_stream,&packet);
    }
    return 0;
}


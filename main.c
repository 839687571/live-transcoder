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



int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);
    
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
        logger(AV_LOG_DEBUG,"segmenter: Unable to find any input streams");
    }

    //logger("Version: %s\n", VERSION);

    avformat_network_init();
    
    struct TranscodeContext ctx;
    

    init_transcoding_context(&ctx,ifmt_ctx->streams[0]);


    struct TranscodeOutput output1;
    init_Transcode_output(&output1);
    
    output1.name="Vid1";
    output1.codec_type=AVMEDIA_TYPE_VIDEO;
    output1.passthrough=false;
    output1.width=352;
    output1.height=240;
    output1.fps=30;
    output1.vid_bitrate=500*1000;
    
    add_output(&ctx,&output1);
    
    struct TranscodeOutput output2;
    init_Transcode_output(&output2);
    output2.name="Vid2";
    output2.codec_type=AVMEDIA_TYPE_VIDEO;
    output2.passthrough=false;
    output2.width=352;
    output2.height=240;
    output2.fps=15;
    output2.vid_bitrate=200*1000;
    add_output(&ctx,&output2);
    


    AVPacket packet;
    av_init_packet(&packet);
    
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        
        if (0!=packet.stream_index) {
            continue;
        }
        convert_packet(&ctx,ifmt_ctx->streams[packet.stream_index],&packet);
        
    }
    return 0;
}


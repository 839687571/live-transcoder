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
#include "input.h"

#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};

#include "TranscodePipeline.h"




/*


bool open_audio_encoder()
{
    AVCodec *codec      = NULL;
    AVCodecContext *vc  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("aac");
    if (!codec) {
        logger(AV_LOG_DEBUG,"Unable to find aac");
        return false;
    }
    vc = avcodec_alloc_context3(codec);
    
    ret = avcodec_open2(vc, codec, NULL);
    
    return true;
}



void sendFrameToEncoder(int i,struct AVCodecContext *pEncoderContext,const AVFrame* pFrame) {
    
    int ret=0;
    
    ret = avcodec_send_frame(pEncoderContext, pFrame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return;
    }
    
    while (ret >= 0) {
        AVPacket *pPacket = av_packet_alloc();
        ret = avcodec_receive_packet(pEncoderContext, pPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            //av_packet_free(pPacket);
            return;
        }
        else if (ret < 0)
        {
            logger(AV_LOG_ERROR,"Error during encoding");
            return;
        }
        logger(AV_LOG_ERROR,"encoded packet output %d, %s, %d\n",i,av_ts2str(pPacket->pts),pPacket->size);
        
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
        
        decodePacket(ctx,&packet);
    
    }
}

*/

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);

    
    
    //logger("Version: %s\n", VERSION);

    avformat_network_init();
    
    struct InputContext inputCtx;
    init_input(&inputCtx,"/Users/guyjacubovski/Sample_video/קישון - תעלת בלאומילך.avi");

    
    struct TranscodeContext ctx;
    init_transcoding_context(&ctx,&inputCtx);


    struct TranscodeOutput output1;
    init_Transcode_output(&output1);
    add_output(&ctx,&output1);
 
    
    process(&inputCtx);
    return 0;
}


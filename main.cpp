#define __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS

extern "C" {
    #define __STDC_CONSTANT_MACROS
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
}

#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#include "logger.hpp"
#include "input.hpp"

#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};

#include "TranscodePipeline.hpp"




/*


#define MAX_INPUTS 10
#define MAX_OUTPUTS 10

struct Context {
    char *pSourceFileName;
    AVFormatContext *ifmt_ctx;
    
    int inputs;
    AVCodec* input_codecs[MAX_INPUTS];
    AVCodecContext* input_codec_contexts[MAX_INPUTS];
    
    int outputs;
    AVCodec* output_codec[MAX_OUTPUTS];
    AVCodecContext* output_codec_contexts[MAX_OUTPUTS];
};


void initContext(struct Context *pContext)
{
    pContext->ifmt_ctx=NULL;
    for (int i=0;i<MAX_INPUTS;i++) {
        pContext->input_codecs[i]=NULL;
        pContext->input_codec_contexts[i]=NULL;
    }
    for (int i=0;i<MAX_OUTPUTS;i++) {
        pContext->output_codec[i]=NULL;
        pContext->output_codec_contexts[i]=NULL;
    }
    pContext->inputs=0;
    pContext->outputs=0;
    
}

bool open_video_encoder(struct Context *pContext,AVCodecContext* pDecoderContext,int width,int height,int bitrate)
{
    AVCodec *codec      = NULL;
    AVCodecContext *enc_ctx  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        logger(AV_LOG_DEBUG,"Unable to find libx264");
        return false;
    }
    enc_ctx = avcodec_alloc_context3(codec);
    enc_ctx->height = height;
    enc_ctx->width = width;
    enc_ctx->sample_aspect_ratio = pDecoderContext->sample_aspect_ratio;
    enc_ctx->pix_fmt = pDecoderContext->pix_fmt;
    enc_ctx->time_base = av_inv_q(pDecoderContext->framerate);
    enc_ctx->bit_rate=bitrate;
    ret = avcodec_open2(enc_ctx, codec, NULL);
    
    pContext->output_codec[pContext->outputs]=codec;
    pContext->output_codec_contexts[pContext->outputs]=enc_ctx;

    pContext->outputs++;
    return true;
}

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

void encodeFrame(struct Context *ctx,const AVFrame* pFrame) {
    
    int ret;

    
    for (int i=0;i<ctx->outputs;i++) {
        logger(AV_LOG_ERROR,"Sending  frame to encoder #%d\n",i);

        AVCodecContext* pEncoderContext=ctx->output_codec_contexts[i];

        sendFrameToEncoder(i,pEncoderContext,pFrame);
        
    }
    
}


void OnInputFrame(struct Context *ctx,AVCodecContext* pDecoderContext,const AVFrame *pFrame)
{
    
    if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
        
        logger(AV_LOG_ERROR,"decoded video: pts=%s (%s), frame type=%s;width=%d;height=%d",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               av_ts2str(pFrame->pkt_duration),
               pict_type_to_string(pFrame->pict_type),pFrame->width,pFrame->height);
        
      //  printf("saving frame %3d\n", pDecoderContext->frame_number);
    } else {
        return;
        logger(AV_LOG_ERROR,"decoded audio: pts=%s (%s);channels=%d;sample rate=%d; length=%d; format=%d ",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               pFrame->channels,pFrame->sample_rate,pFrame->nb_samples,pFrame->format);
        
        return;
        
    }

    encodeFrame(ctx,pFrame);
}



void decodePacket(struct Context *ctx,const AVPacket* pkt) {

    int ret;
    
    int stream_index = pkt->stream_index;
    
    logger(AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u",stream_index);
    
    AVCodecContext* pDecoderContext=ctx->input_codec_contexts[stream_index];

    ret = avcodec_send_packet(pDecoderContext, pkt);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Error sending a packet for decoding");
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
            logger(AV_LOG_ERROR,"Error during decoding");
            return;
        }
        pFrame->pts = pFrame->best_effort_timestamp;

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
    //add_output(&ctx,&output1);
 
    
    process(&inputCtx);
    return 0;
}


//
//  TranscoderEncoder.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 03/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "codec.h"
#include "logger.h"

#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

static  AVRational standard_timebase = {1,1000};

int init_decoder(struct TranscoderCodecContext * pContext,AVCodecParameters *pCodecParams)
{
    
    AVCodec *dec = pContext->codec=avcodec_find_decoder(pCodecParams->codec_id);
    AVCodecContext *codec_ctx;
    if (!dec) {
        LOGGER0(CATEGORY_CODEC,AV_LOG_ERROR, "Failed to find decoder for stream");
        return AVERROR_DECODER_NOT_FOUND;
    }
    codec_ctx = avcodec_alloc_context3(dec);
    if (!codec_ctx) {
        LOGGER0(CATEGORY_CODEC,AV_LOG_ERROR, "Failed to allocate the decoder context for stream");
        return AVERROR(ENOMEM);
    }
    codec_ctx->time_base=standard_timebase;

    
    int ret = avcodec_parameters_to_context(codec_ctx, pCodecParams);
    if (ret < 0) {
        LOGGER(CATEGORY_CODEC, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context for stream  %d (%s)",ret,av_err2str(ret));
        return ret;
    }

    /* Reencode video & audio and remux subtitles etc. */
    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            //codec_ctx->framerate = av_guess_frame_rate(pInputStream->->ifmt_ctx, pInputStream, NULL);
        }
        /* Open decoder */
        ret = avcodec_open2(codec_ctx, dec, NULL);
        if (ret < 0) {
            LOGGER( CATEGORY_CODEC, AV_LOG_ERROR, "Failed to open decoder for stream %d (%s)",ret,av_err2str(ret));
            return ret;
        }
    }
    pContext->ctx = codec_ctx;
    
    return 0;
}


int init_video_encoder(struct TranscoderCodecContext * pContext,
                       AVRational inputAspectRatio,
                       enum AVPixelFormat inputPixelFormat,
                       AVRational inputTimeBase,
                       int width,int height,int bitrate)
{
    AVCodec *codec      = NULL;
    AVCodecContext *enc_ctx  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        LOGGER0(CATEGORY_CODEC,AV_LOG_ERROR,"Unable to find libx264");
        return ret;
    }
    enc_ctx = avcodec_alloc_context3(codec);
    enc_ctx->height = height;
    enc_ctx->width = width;
    enc_ctx->sample_aspect_ratio = inputAspectRatio;
    enc_ctx->pix_fmt = inputPixelFormat;
    enc_ctx->bit_rate=bitrate*1000;
    //enc_ctx->rc_min_rate=bitrate*0.8;
    //enc_ctx->rc_max_rate=bitrate*1.2;
    enc_ctx->rc_buffer_size=4*bitrate/30;
    enc_ctx->gop_size=60;
    enc_ctx->qmin = 10;
    enc_ctx->qmax = 51;
    enc_ctx->time_base=standard_timebase;

    av_opt_set(enc_ctx->priv_data, "preset", "veryfast", 0);
    av_opt_set(enc_ctx->priv_data, "tune", "zerolatency", 0);
    av_opt_set(enc_ctx->priv_data, "profile", "baseline", 0);
    
    ret = avcodec_open2(enc_ctx, codec,NULL);
    if (ret<0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_ERROR,"error initilizing video encoder %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    
    pContext->codec=codec;
    pContext->ctx=enc_ctx;
    LOGGER(CATEGORY_CODEC,AV_LOG_INFO,"video encoder  %dx%d %d Kbit/s initilaized",width,height,bitrate);

    return 0;
}

int init_audio_encoder(struct TranscoderCodecContext * pContext,struct TranscoderFilter* pFilter)
{
    AVCodec *codec      = NULL;
    AVCodecContext *enc_ctx  = NULL;
    int ret = 0;
    
    
    codec = avcodec_find_encoder_by_name("aac");
    if (!codec) {
        LOGGER0(CATEGORY_CODEC,AV_LOG_ERROR,"Unable to find aac");
        return -1;
    }
    enc_ctx = avcodec_alloc_context3(codec);
    
    enc_ctx->sample_fmt = av_buffersink_get_format(pFilter->sink_ctx);
    enc_ctx->channel_layout = av_buffersink_get_channel_layout(pFilter->sink_ctx);
    enc_ctx->channels = av_buffersink_get_channels(pFilter->sink_ctx);
    enc_ctx->sample_rate = av_buffersink_get_sample_rate(pFilter->sink_ctx);
    enc_ctx->time_base = av_buffersink_get_time_base(pFilter->sink_ctx);
    
    ret = avcodec_open2(enc_ctx, codec,NULL);
    if (ret<0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_ERROR,"error initilizing video encoder %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    av_buffersink_set_frame_size(pFilter->sink_ctx, enc_ctx->frame_size);

    pContext->codec=codec;
    pContext->ctx=enc_ctx;
    LOGGER(CATEGORY_CODEC,AV_LOG_INFO,"audio encoder  %dKhz %d Kbit/s initilaized",enc_ctx->sample_rate ,0);

    return 0;
}

int send_encode_frame(struct TranscoderCodecContext *encoder,const AVFrame* pFrame)
{
    int ret = avcodec_send_frame(encoder->ctx, pFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return 0;
    }
    if (ret < 0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_WARNING, "Error sending a packet for encoding %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    return ret;
}
int receive_encoder_packet(struct TranscoderCodecContext *encoder,AVPacket* pkt)
{
    int ret;
    ret = avcodec_receive_packet(encoder->ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return ret;
    }
    if (ret<0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_WARNING, "Error recveiving a packet for encoding %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    
    return ret;
    
}


int send_decoder_packet(struct TranscoderCodecContext *decoder,const AVPacket* pkt) {
    
    int ret;
    
    LOGGER0(CATEGORY_CODEC, AV_LOG_DEBUG,"Sending packget to decoder");
    
    
    ret = avcodec_send_packet(decoder->ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return 0;
    }
    if (ret < 0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_ERROR, "Error sending a packet to decoder %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    
    return 0;
    
}


int receive_decoder_frame(struct TranscoderCodecContext *decoder,AVFrame *pFrame)
{
    int ret;
    ret = avcodec_receive_frame(decoder->ctx, pFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return ret;
    }
    if (ret<0) {
        LOGGER(CATEGORY_CODEC,AV_LOG_ERROR, "Error recieving packet from decoder %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    //pFrame->pts = pFrame->best_effort_timestamp;
        

    return 0;
}


//
//  TranscoderEncoder.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 03/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "codec.h"


int init_decoder(struct TranscoderCodecContext * pContext,AVStream *pInputStream)
{
    
    const AVCodecParameters *params=pInputStream->codecpar;
    AVCodec *dec = pContext->codec=avcodec_find_decoder(params->codec_id);
    AVCodecContext *codec_ctx;
    if (!dec) {
        logger(AV_LOG_ERROR, "Failed to find decoder for stream #%u",pInputStream->index);
        return AVERROR_DECODER_NOT_FOUND;
    }
    codec_ctx = avcodec_alloc_context3(dec);
    if (!codec_ctx) {
        logger(AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u", pInputStream->index);
        return AVERROR(ENOMEM);
    }
    int ret = avcodec_parameters_to_context(codec_ctx, params);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
               "for stream #%u", pInputStream->index);
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
            logger( AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", pInputStream->index);
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
        logger(AV_LOG_DEBUG,"Unable to find libx264");
        return -1;
    }
    enc_ctx = avcodec_alloc_context3(codec);
    enc_ctx->height = height;
    enc_ctx->width = width;
    enc_ctx->sample_aspect_ratio = inputAspectRatio;
    enc_ctx->pix_fmt = inputPixelFormat;
    enc_ctx->time_base = inputTimeBase;
    enc_ctx->bit_rate=bitrate;
    ret = avcodec_open2(enc_ctx, codec, NULL);
    if (ret<0) {
        logger(AV_LOG_DEBUG,"error initilizing video encoder %d",ret);
        return -1;
    }
    
    pContext->codec=codec;
    pContext->ctx=enc_ctx;
    return 0;
}

int init_audio_encoder(struct TranscoderCodecContext * pContext)
{
    return 0;
}

int send_encode_frame(struct TranscoderCodecContext *encoder,const AVFrame* pFrame)
{
    int ret = avcodec_send_frame(encoder->ctx, pFrame);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Error sending a packet for encoding");
        exit(1);
    }
    return ret;
}
int receive_encoder_packet(struct TranscoderCodecContext *encoder,AVPacket* pkt)
{
    int ret;
    ret = avcodec_receive_packet(encoder->ctx, pkt);
    if (ret<0) {
        return ret;
    }
    
    return 0;
    
}


int send_decoder_packet(struct TranscoderCodecContext *decoder,const AVPacket* pkt) {
    
    int ret;
    
    int stream_index = pkt->stream_index;
    
    logger(AV_LOG_DEBUG, "Decoder gave frame of stream_index %u",stream_index);
    
    
    ret = avcodec_send_packet(decoder->ctx, pkt);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Error sending a packet for decoding");
        exit(1);
    }
    
    return 0;
    
}


int receive_decoder_frame(struct TranscoderCodecContext *decoder,AVFrame *pFrame)
{
    int ret;
    ret = avcodec_receive_frame(decoder->ctx, pFrame);
    if (ret<0) {
        return ret;
    }
    pFrame->pts = pFrame->best_effort_timestamp;
        

    return 0;
}


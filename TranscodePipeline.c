//
//  TranscodePipeline.cpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "TranscodePipeline.h"
#include "logger.h"

    



int init_decoder(struct TranscodeContext * pContext,AVStream *pInputStream) {
    
    AVCodec *dec = pContext->input_codecs[pContext->inputs]=avcodec_find_decoder(pInputStream->codecpar->codec_id);
    AVCodecContext *codec_ctx;
    if (!dec) {
        logger(AV_LOG_ERROR, "Failed to find decoder for stream #%u", pContext->inputs);
        return AVERROR_DECODER_NOT_FOUND;
    }
    codec_ctx = avcodec_alloc_context3(dec);
    if (!codec_ctx) {
        logger(AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u", pContext->inputs);
        return AVERROR(ENOMEM);
    }
    int ret = avcodec_parameters_to_context(codec_ctx, pInputStream->codecpar);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
               "for stream #%u", pContext->inputs);
        return ret;
    }
    /* Reencode video & audio and remux subtitles etc. */
    if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
            codec_ctx->framerate = av_guess_frame_rate(pContext->pInputContext->ifmt_ctx, pInputStream, NULL);
        /* Open decoder */
        ret = avcodec_open2(codec_ctx, dec, NULL);
        if (ret < 0) {
            logger( AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", pContext->inputs);
            return ret;
        }
    }
    pContext->input_codec_contexts[pContext->inputs] = codec_ctx;
    
    pContext->inputs++;
    return 0;
}



int OnInputFrame(struct TranscodeContext *pContext,AVCodecContext* pDecoderContext,const AVFrame *pFrame)
{
    
    if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
        
        logger(AV_LOG_ERROR,"decoded video: pts=%s (%s), frame type=%s;width=%d;height=%d",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               pict_type_to_string(pFrame->pict_type),pFrame->width,pFrame->height);
        
        //  printf("saving frame %3d\n", pDecoderContext->frame_number);
    } else {
        logger(AV_LOG_ERROR,"decoded audio: pts=%s (%s);channels=%d;sample rate=%d; length=%d; format=%d ",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               pFrame->channels,pFrame->sample_rate,pFrame->nb_samples,pFrame->format);
        
        return 0;
        
    }
    
    for (int i=0;i<pContext->filters;i++) {
        struct TranscodeFilter *pFilter=&pContext->filter[i];
        send_filter_frame(pFilter,pFrame);
        
        int ret=0;
        while (ret >= 0) {
            AVFrame *pOutFrame = av_frame_alloc();
            
            ret = receive_filter_frame(pFilter,pOutFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_frame_free(&pOutFrame);
                return 0;
            }
            else if (ret < 0)
            {
                logger(AV_LOG_ERROR,"Error during decoding");
                return -1;
            }
            
            logger(AV_LOG_ERROR,"filtered video: pts=%s (%s), frame type=%s;width=%d;height=%d",
                   av_ts2str(pOutFrame->pts), av_ts2timestr(pOutFrame->pts, &pDecoderContext->time_base),
                   pict_type_to_string(pOutFrame->pict_type),pOutFrame->width,pOutFrame->height);
            
            //encodeFrame(pContext,pFrame);
            av_frame_free(&pOutFrame);
        }
    }
    return 0;
}

int decodePacket(struct TranscodeContext *transcodingContext,struct InputContext *inputContext,const AVPacket* pkt) {
    
    int ret;
    
    int stream_index = pkt->stream_index;
    
    logger(AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u",stream_index);
    
    AVCodecContext* pDecoderContext=transcodingContext->input_codec_contexts[stream_index];
    
    ret = avcodec_send_packet(pDecoderContext, pkt);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Error sending a packet for decoding");
        exit(1);
    }
    
    while (ret >= 0) {
        AVFrame *pFrame = av_frame_alloc();
        
        ret = avcodec_receive_frame(pDecoderContext, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&pFrame);
            return 0;
        }
        else if (ret < 0)
        {
            logger(AV_LOG_ERROR,"Error during decoding");
            return -1;
        }
        pFrame->pts = pFrame->best_effort_timestamp;
        
        OnInputFrame(transcodingContext,pDecoderContext,pFrame);
        
        av_frame_free(&pFrame);
    }
    return 0;
}

int on_packet_cb(struct TranscodeContext *pContext,struct InputContext *ctx,struct AVStream* pStream, struct AVPacket* packet)
{
    for (int i=0;i<pContext->outputs;i++) {
        struct TranscodeOutput *pOutput=pContext->output[i];
        if ((pStream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO && pOutput->vid_passthrough) ||
            (pStream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO && pOutput->aud_passthrough) ||
            pStream->codecpar->codec_type==AVMEDIA_TYPE_DATA) {
            send_output_packet(pOutput,packet);
        }
    }
    return decodePacket(pContext,ctx,packet);
}



int init_transcoding_context(struct TranscodeContext *pContext,struct InputContext* pInputContext)
{
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
    
    
    pInputContext->pTranscodingContext=pContext;
    pContext->pInputContext=pInputContext;
    pContext->filters=0;
    
    
    for (int i = 0; i < pInputContext->ifmt_ctx->nb_streams; i++) {
        AVStream *pStream = pInputContext->ifmt_ctx->streams[i];
        
        init_decoder(pContext,pStream);
        
        AVCodecContext *pDecoderContext=pContext->input_codec_contexts[pContext->inputs-1];
        
        struct TranscoderFilter* pFilter=&pContext->filter[pContext->filters++];
        if (pStream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            init_filter(pFilter,pStream,pDecoderContext,"scale=100x100:force_original_aspect_ratio=decrease");
        }

        //open_video_encoder(pContext,codec_ctx,codec_ctx->width,codec_ctx->height,1000*1000);
        //open_video_encoder(pContext,codec_ctx,codec_ctx->width,codec_ctx->height,300*1000);
        

    }
    pInputContext->pOnPacketCB=on_packet_cb;
    return 0;
}

int add_output(struct TranscodeContext* pContext, struct TranscodeOutput * pOutput)
{
    pContext->output[pContext->outputs++]=pOutput;
    return 0;
}


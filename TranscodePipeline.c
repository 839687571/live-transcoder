//
//  TranscodePipeline.cpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "TranscodePipeline.h"
#include "logger.h"

    


int encodeFrame(struct TranscodeContext *pContext,AVFrame *pFrame) {
 
    int ret=0;
    
    struct TranscoderCodecContext* pEncoder=&pContext->encoder[0];
    ret=send_encode_frame(pEncoder,pFrame);
    
    while (ret >= 0) {
        AVPacket *pOutPacket = av_packet_alloc();
        
        ret = receive_encoder_packet(pEncoder,pOutPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_packet_free(&pOutPacket);
            return 0;
        }
        else if (ret < 0)
        {
            logger(AV_LOG_ERROR,"Error during decoding");
            return -1;
        }
        
        logger(AV_LOG_ERROR,"encoded frame: pts=%s (%s) size=%d",
               av_ts2str(pOutPacket->pts), av_ts2timestr(pOutPacket->pts, &pEncoder->ctx->time_base),
               pOutPacket->size);
        
        av_packet_free(&pOutPacket);
    }
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
            
            encodeFrame(pContext,pFrame);
            av_frame_free(&pOutFrame);
        }
    }
    return 0;
}

int decodePacket(struct TranscodeContext *transcodingContext,struct InputContext *inputContext,const AVPacket* pkt) {
    
    int ret;
    
    int stream_index = pkt->stream_index;
    
    logger(AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u",stream_index);
    
    struct TranscoderCodecContext* pDecoder=&transcodingContext->decoder[stream_index];
    
    
    

    ret = send_decoder_packet(pDecoder, pkt);
    if (ret < 0) {
        logger(AV_LOG_ERROR, "Error sending a packet for decoding");
        exit(1);
    }
    
    while (ret >= 0) {
        AVFrame *pFrame = av_frame_alloc();
        
        ret = receive_decoder_frame(pDecoder, pFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&pFrame);
            return 0;
        }
        else if (ret < 0)
        {
            logger(AV_LOG_ERROR,"Error during decoding");
            return -1;
        }
        OnInputFrame(transcodingContext,pDecoder->ctx,pFrame);
        
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
    pContext->inputs=0;
    pContext->outputs=0;
    
    
    pInputContext->pTranscodingContext=pContext;
    pContext->pInputContext=pInputContext;
    pContext->filters=0;
    
    
    for (int i = 0; i < pInputContext->ifmt_ctx->nb_streams; i++) {
        AVStream *pStream = pInputContext->ifmt_ctx->streams[i];
        
        struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[i];
        init_decoder(pDecoderContext,pStream);
        
        
        struct TranscoderFilter* pFilter=&pContext->filter[pContext->filters++];
        if (pStream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            init_filter(pFilter,pStream,pDecoderContext->ctx,"scale=512x384:force_original_aspect_ratio=decrease");
            struct TranscoderCodecContext* pCodec=&pContext->encoder[pContext->encoders++];

            int width=av_buffersink_get_w(pFilter->sink_ctx);
            int height=av_buffersink_get_h(pFilter->sink_ctx);
            AVRational frameRate=pStream->avg_frame_rate;
            
            enum AVPixelFormat format= av_buffersink_get_format(pFilter->sink_ctx);
            init_video_encoder(pCodec, pDecoderContext->ctx->sample_aspect_ratio,format,frameRate,width,height,1000*1000);
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


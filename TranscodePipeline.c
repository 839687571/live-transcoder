//
//  TranscodePipeline.cpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "TranscodePipeline.h"
#include "logger.h"


int init_transcoding_context(struct TranscodeContext *pContext,struct AVStream* pStream)
{
    pContext->inputs=0;
    pContext->outputs=0;
    pContext->filters=0;
    pContext->inputStream=pStream;
    
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    init_decoder(pDecoderContext,pStream);
    
    return 0;
}



int encodeFrame(struct TranscodeContext *pContext,int encoderId,int outputId,AVFrame *pFrame) {
 

    int ret=0;
    
    struct TranscoderCodecContext* pEncoder=&pContext->encoder[encoderId];
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
            logger(CATEGORY_DEFAULT, AV_LOG_ERROR,"Error during decoding");
            return -1;
        }
        
        logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[%d] encoded frame for output %d: pts=%s (%s) size=%d",
               encoderId,
               outputId,
               av_ts2str(pOutPacket->pts), av_ts2timestr(pOutPacket->pts, &pEncoder->ctx->time_base),
               pOutPacket->size);
        
        
        send_output_packet(pContext->output[outputId],pOutPacket);
        
        av_packet_free(&pOutPacket);
    }
    return 0;
}

int OnDecodedFrame(struct TranscodeContext *pContext,AVCodecContext* pDecoderContext,const AVFrame *pFrame)
{
    
    if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
        
        logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"decoded video: pts=%s (%s), frame type=%s;width=%d;height=%d",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               pict_type_to_string(pFrame->pict_type),pFrame->width,pFrame->height);
        
        //return 0;
        //  printf("saving frame %3d\n", pDecoderContext->frame_number);
    } else {
        logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"decoded audio: pts=%s (%s);channels=%d;sample rate=%d; length=%d; format=%d ",
               av_ts2str(pFrame->pts), av_ts2timestr(pFrame->pts, &pDecoderContext->time_base),
               pFrame->channels,pFrame->sample_rate,pFrame->nb_samples,pFrame->format);
        
        
    }
    
    for (int filterId=0;filterId<pContext->filters;filterId++) {
        struct TranscodeFilter *pFilter=&pContext->filter[filterId];
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
                logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Error during decoding");
                return -1;
            }
            
            if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
                logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"filtered video: pts=%s (%s), frame type=%s;width=%d;height=%d",
                       av_ts2str(pOutFrame->pts), av_ts2timestr(pOutFrame->pts, &pDecoderContext->time_base),
                       pict_type_to_string(pOutFrame->pict_type),pOutFrame->width,pOutFrame->height);
            }
            
            if (pDecoderContext->codec_type==AVMEDIA_TYPE_AUDIO) {
                logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"filtered audio: pts=%s (%s), size=%d",
                       av_ts2str(pOutFrame->pts), av_ts2timestr(pOutFrame->pts, &pDecoderContext->time_base),pOutFrame->nb_samples);
            }
            
            for (int outputId=0;outputId<pContext->outputs;outputId++) {
                struct TranscodeOutput *pOutput=pContext->output[outputId];
                if (pOutput->filterId==filterId){
                    logger(CATEGORY_DEFAULT,AV_LOG_DEBUG,"sending frame from filterId %d to encoderId %d for output %s",filterId,pOutput->encoderId,pOutput->name);
                    encodeFrame(pContext,pOutput->encoderId,outputId,pFrame);
                }
            }
            av_frame_free(&pOutFrame);
        }
    }
    return 0;
}

int decodePacket(struct TranscodeContext *transcodingContext,const AVPacket* pkt) {
    
    int ret;
    
    int stream_index = pkt->stream_index;
    
    logger(CATEGORY_DEFAULT,AV_LOG_DEBUG, "Send packet from stream_index %u to decoder",stream_index);
    
    struct TranscoderCodecContext* pDecoder=&transcodingContext->decoder[stream_index];
    

    ret = send_decoder_packet(pDecoder, pkt);
    if (ret < 0) {
        logger(CATEGORY_DEFAULT,AV_LOG_ERROR, "Error sending a packet for decoding");
        return ret;
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
            logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Error during decoding");
            return ret;
        }
        OnDecodedFrame(transcodingContext,pDecoder->ctx,pFrame);
        
        av_frame_free(&pFrame);
    }
    return 0;
}

int convert_packet(struct TranscodeContext *pContext ,struct AVPacket* packet)
{
    bool shouldDecode=false;
    for (int i=0;i<pContext->outputs;i++) {
        struct TranscodeOutput *pOutput=pContext->output[i];
        if (pOutput->codec_type==pContext->inputStream->codecpar->codec_type)
        {
            if (pOutput->passthrough)
            {
                send_output_packet(pOutput,packet);
            }
            else
            {
                shouldDecode=true;
            }
        }
    }
    if (shouldDecode) {
       return decodePacket(pContext,packet);
    }
    return 0;
}

struct TranscoderFilter* GetFilter(struct TranscodeContext* pContext,struct TranscodeOutput* pOutput,struct TranscoderCodecContext *pDecoderContext,const char* config)
{
    struct AVStream*  pStream= pContext->inputStream;

    struct TranscoderFilter* pFilter=NULL;
    pOutput->filterId=-1;
    for (int selectedFilter=0; selectedFilter<pContext->filters;selectedFilter++) {
        pFilter=&pContext->filter[selectedFilter];
        if (strcmp(pFilter->config,config)==0) {
            pOutput->filterId=selectedFilter;
            logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Resuing existing filter %s",pOutput->name,config);
        }
    }
    if ( pOutput->filterId==-1) {
        pOutput->filterId=pContext->filters++;
        pFilter=&pContext->filter[pOutput->filterId];
        int ret=init_filter(pFilter,pStream,pDecoderContext->ctx,config);
        if (ret<0) {
            logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Cannot create filter %s",pOutput->name,config);
            return NULL;
        }
        logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Created new  filter %s",pOutput->name,config);
    }
    return pFilter;
}

int add_output(struct TranscodeContext* pContext, struct TranscodeOutput * pOutput)
{
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    struct AVStream*  pStream= pContext->inputStream;
    
    if (!pOutput->passthrough) {
        char filterConfig[2048];
        
        if (pStream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            sprintf(filterConfig,"scale=%dx%d:force_original_aspect_ratio=decrease",pOutput->videoParams.width,pOutput->videoParams.height);
            struct TranscoderFilter* pFilter=GetFilter(pContext,pOutput,pDecoderContext,filterConfig);
            
            pOutput->encoderId=pContext->encoders++;
            struct TranscoderCodecContext* pCodec=&pContext->encoder[pOutput->encoderId];
            
            int width=av_buffersink_get_w(pFilter->sink_ctx);
            int height=av_buffersink_get_h(pFilter->sink_ctx);
            AVRational frameRate=pStream->avg_frame_rate;
            
            enum AVPixelFormat format= av_buffersink_get_format(pFilter->sink_ctx);
            init_video_encoder(pCodec, pDecoderContext->ctx->sample_aspect_ratio,format,frameRate,width,height,pOutput->bitrate*1000);
            logger(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Added encoder %d bitrate=%d",pOutput->name,pOutput->encoderId,pOutput->bitrate*1000);
        }
        
        if (pStream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            sprintf(filterConfig,"aformat=sample_fmts=fltp:channel_layouts=stereo:sample_rates=%d",pOutput->audioParams.samplingRate);
            struct TranscoderFilter* pFilter=GetFilter(pContext,pOutput,pDecoderContext,filterConfig);
            pOutput->encoderId=pContext->encoders++;

            struct TranscoderCodecContext* pCodec=&pContext->encoder[pOutput->encoderId];
            
            init_audio_encoder(pCodec, pFilter);
        }
    }
    
    pContext->output[pContext->outputs++]=pOutput;
    return 0;
}


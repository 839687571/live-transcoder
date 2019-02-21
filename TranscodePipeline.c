//
//  TranscodePipeline.cpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "TranscodePipeline.h"
#include "logger.h"


int init_transcoding_context(struct TranscodeContext *pContext,struct AVCodecParameters* codecParams)
{
    pContext->inputs=0;
    pContext->outputs=0;
    pContext->filters=0;
    pContext->inputCodecParams=codecParams;
    
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    init_decoder(pDecoderContext,codecParams);
    
    return 0;
}



int encodeFrame(struct TranscodeContext *pContext,int encoderId,int outputId,AVFrame *pFrame) {
 

    LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG, "Sending packet  to encoder pts=%s",ts2str(pFrame->pts,true))
    
    
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
            LOGGER(CATEGORY_DEFAULT, AV_LOG_ERROR,"Error during decoding %d (%s)",ret,av_err2str(ret))
            return ret;
        }
        
        LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[%d] encoded frame for output %d: pts=%s  size=%d",
               encoderId,
               outputId,
               ts2str(pOutPacket->pts,true),
               pOutPacket->size);
        
        
        send_output_packet(pContext->output[outputId],pOutPacket);
        
        av_packet_free(&pOutPacket);
    }
    return 0;
}

int sendFrameToFilter(struct TranscodeContext *pContext,int filterId, AVCodecContext* pDecoderContext, AVFrame *pFrame)
{
    struct TranscodeFilter *pFilter=&pContext->filter[filterId];
    LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"sending frame to filter (%d): pts=%s",
           filterId,
           ts2str(pFrame->pts,true));
    
    int ret=send_filter_frame(pFilter,pFrame);
    if (ret<0) {
        
        LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"failed sending frame to filter (%d): pts=%s %d (%s)",
               filterId,
               ts2str(pFrame->pts,true),
               ret,
               av_err2str(ret));
    }
    
    while (ret >= 0) {
        AVFrame *pOutFrame = av_frame_alloc();
        
        
        ret = receive_filter_frame(pFilter,pOutFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&pOutFrame);
            return 0;
        }
        else if (ret < 0)
        {
            return ret;
        }
        
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"filtered video(%d): pts=%s, frame type=%s;width=%d;height=%d",
                   filterId,
                   ts2str(pOutFrame->pts,true),
                   pict_type_to_string(pOutFrame->pict_type),pOutFrame->width,pOutFrame->height);
        }
        
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_AUDIO) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"filtered audio(%d): pts=%s, size=%d",
                   filterId,
                   ts2str(pOutFrame->pts,true),pOutFrame->nb_samples);
        }
        
        for (int outputId=0;outputId<pContext->outputs;outputId++) {
            struct TranscodeOutput *pOutput=pContext->output[outputId];
            if (pOutput->filterId==filterId){
                LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"sending frame from filterId %d to encoderId %d for output %s",filterId,pOutput->encoderId,pOutput->name);
                encodeFrame(pContext,pOutput->encoderId,outputId,pOutFrame);
            }
        }
        av_frame_free(&pOutFrame);
    }
    return 0;
}

int OnDecodedFrame(struct TranscodeContext *pContext,AVCodecContext* pDecoderContext, AVFrame *pFrame)
{

    if (pFrame!=NULL) {
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
            
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"decoded video: pts=%s, frame type=%s;width=%d;height=%d",
                   ts2str(pFrame->pts,true),
                   pict_type_to_string(pFrame->pict_type),pFrame->width,pFrame->height);
            
            //return 0;
            //  printf("saving frame %3d\n", pDecoderContext->frame_number);
        } else {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"decoded audio: pts=%s;channels=%d;sample rate=%d; length=%d; format=%d ",
                   ts2str(pFrame->pts,true),
                   pFrame->channels,pFrame->sample_rate,pFrame->nb_samples,pFrame->format);
            
            
        }
    }
    for (int filterId=0;filterId<pContext->filters;filterId++) {
        
        sendFrameToFilter(pContext,filterId,pDecoderContext,pFrame);
       
    }
    
    for (int outputId=0;outputId<pContext->outputs;outputId++) {
        struct TranscodeOutput *pOutput=pContext->output[outputId];
        if (pOutput->filterId==-1 && pOutput->encoderId!=-1){
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"sending frame directly from decoderto encoderId %d for output %s",pOutput->encoderId,pOutput->name);
            encodeFrame(pContext,pOutput->encoderId,outputId,pFrame);
        }
    }
    
    return 0;
}

int decodePacket(struct TranscodeContext *transcodingContext,const AVPacket* pkt) {
    
    int ret;
    
    
    if (pkt!=NULL) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG, "Sending packet  to decoder pts=%s dts=%s",
               ts2str(pkt->pts,true),
               ts2str(pkt->dts,true));
    }
    struct TranscoderCodecContext* pDecoder=&transcodingContext->decoder[0];
    

    ret = send_decoder_packet(pDecoder, pkt);
    if (ret < 0) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR, "Error sending a packet for decoding %d (%s)",ret,av_err2str(ret));
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
            LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"Error during decoding %d (%s)",ret,av_err2str(ret));
            return ret;
        }
        OnDecodedFrame(transcodingContext,pDecoder->ctx,pFrame);
        
        av_frame_free(&pFrame);
    }
    
    //drain mode
    if (pkt==NULL) {
        OnDecodedFrame(transcodingContext,pDecoder->ctx,NULL);
    }
    return 0;
}

int convert_packet(struct TranscodeContext *pContext ,struct AVPacket* packet)
{
    bool shouldDecode=false;
    for (int i=0;i<pContext->outputs;i++) {
        struct TranscodeOutput *pOutput=pContext->output[i];
        if (pOutput->passthrough)
        {
            send_output_packet(pOutput,packet);
        }
        else
        {
            shouldDecode=true;
        }
    }
    if (shouldDecode) {
       return decodePacket(pContext,packet);
    }
    return 0;
}

struct TranscoderFilter* GetFilter(struct TranscodeContext* pContext,struct TranscodeOutput* pOutput,struct TranscoderCodecContext *pDecoderContext,const char* config)
{
    struct TranscoderFilter* pFilter=NULL;
    pOutput->filterId=-1;
    for (int selectedFilter=0; selectedFilter<pContext->filters;selectedFilter++) {
        pFilter=&pContext->filter[selectedFilter];
        if (strcmp(pFilter->config,config)==0) {
            pOutput->filterId=selectedFilter;
            LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Resuing existing filter %s",pOutput->name,config);
        }
    }
    if ( pOutput->filterId==-1) {
        pOutput->filterId=pContext->filters++;
        pFilter=&pContext->filter[pOutput->filterId];
        int ret=init_filter(pFilter,pDecoderContext->ctx,config);
        if (ret<0) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Cannot create filter %s",pOutput->name,config);
            return NULL;
        }
        LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Created new  filter %s",pOutput->name,config);
    }
    return pFilter;
}

int add_output(struct TranscodeContext* pContext, struct TranscodeOutput * pOutput)
{
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    pContext->output[pContext->outputs++]=pOutput;
    int ret=0;

    if (!pOutput->passthrough) {
        char filterConfig[2048];
        
        if (pOutput->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            sprintf(filterConfig,"scale=w=%d:h=%d",pOutput->videoParams.width,pOutput->videoParams.height);
            struct TranscoderFilter* pFilter=GetFilter(pContext,pOutput,pDecoderContext,filterConfig);
            
            pOutput->encoderId=pContext->encoders++;
            struct TranscoderCodecContext* pCodec=&pContext->encoder[pOutput->encoderId];
            
            AVRational frameRate={1,30};
            int width=pDecoderContext->ctx->width;
            int height=pDecoderContext->ctx->height;
            enum AVPixelFormat picFormat=pDecoderContext->ctx->pix_fmt;
            
            if (pFilter) {
                
                width=av_buffersink_get_w(pFilter->sink_ctx);
                height=av_buffersink_get_h(pFilter->sink_ctx);
                picFormat= av_buffersink_get_format(pFilter->sink_ctx);
            }
            
            init_video_encoder(pCodec,
                               pDecoderContext->ctx->sample_aspect_ratio,
                               picFormat,
                               frameRate,
                               pOutput,
                               width,
                               height);

            
            LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Added encoder %d bitrate=%d",pOutput->name,pOutput->encoderId,pOutput->bitrate*1000);
            
            struct AVCodecParameters* pCodecParams=avcodec_parameters_alloc();
            avcodec_parameters_from_context(pCodecParams,pCodec->ctx);
            set_output_format(pOutput,pCodecParams);
            
        }
        
        if (pOutput->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            sprintf(filterConfig,"aformat=sample_fmts=fltp:channel_layouts=stereo:sample_rates=%d",pOutput->audioParams.samplingRate);
            struct TranscoderFilter* pFilter=GetFilter(pContext,pOutput,pDecoderContext,filterConfig);
            pOutput->encoderId=pContext->encoders++;

            struct TranscoderCodecContext* pCodec=&pContext->encoder[pOutput->encoderId];
            
            init_audio_encoder(pCodec, pFilter);
            struct AVCodecParameters* pCodecParams=avcodec_parameters_alloc();
            avcodec_parameters_from_context(pCodecParams,pCodec->ctx);
            set_output_format(pOutput,pCodecParams);
        }
    } else
    {
        set_output_format(pOutput,pContext->inputCodecParams);
        
    }
    
    return 0;
}

int close_transcoding_context(struct TranscodeContext *pContext) {
    
    convert_packet(pContext,NULL);

    return 0;
}

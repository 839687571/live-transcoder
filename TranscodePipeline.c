//
//  TranscodePipeline.cpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "TranscodePipeline.h"
#include "utils.h"
#include "logger.h"
#include "config.h"


/* initialization */
int init_transcoding_context(struct TranscodeContext *pContext,struct AVCodecParameters* codecParams)
{
    pContext->inputs=0;
    pContext->outputs=0;
    pContext->filters=0;
    pContext->encoders=0;
    pContext->inputCodecParams=codecParams;
    
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    init_decoder(pDecoderContext,codecParams);
    
    return 0;
}


void get_filter_config(char *filterConfig, struct TranscoderCodecContext *pDecoderContext, struct TranscodeOutput *pOutput)
{
    if (pOutput->codec_type==AVMEDIA_TYPE_VIDEO)
    {
        sprintf(filterConfig,"%s=w=%d:h=%d",
                pDecoderContext->nvidiaAccelerated ? "scale_npp" : "scale",
                pOutput->videoParams.width,
                pOutput->videoParams.height);
    }
    if (pOutput->codec_type==AVMEDIA_TYPE_AUDIO)
    {
        sprintf(filterConfig,"aformat=sample_fmts=fltp:channel_layouts=stereo:sample_rates=%d",pOutput->audioParams.samplingRate);
    }
}

struct TranscoderFilter* GetFilter(struct TranscodeContext* pContext,struct TranscodeOutput* pOutput,struct TranscoderCodecContext *pDecoderContext)
{
    char filterConfig[2048];
    get_filter_config(filterConfig, pDecoderContext, pOutput);
    
    struct TranscoderFilter* pFilter=NULL;
    pOutput->filterId=-1;
    for (int selectedFilter=0; selectedFilter<pContext->filters;selectedFilter++) {
        pFilter=&pContext->filter[selectedFilter];
        if (strcmp(pFilter->config,filterConfig)==0) {
            pOutput->filterId=selectedFilter;
            LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Resuing existing filter %s",pOutput->name,filterConfig);
        }
    }
    if ( pOutput->filterId==-1) {
        pFilter=&pContext->filter[pContext->filters];
        int ret=init_filter(pFilter,pDecoderContext->ctx,filterConfig);
        if (ret<0) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"Output %s - Cannot create filter %s",pOutput->name,filterConfig);
            return NULL;
        }
        
        pOutput->filterId=pContext->filters++;
        LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Created new  filter %s",pOutput->name,filterConfig);
    }
    return pFilter;
}



int config_encoder(struct TranscodeOutput *pOutput, struct TranscoderCodecContext *pDecoderContext, struct TranscoderFilter *pFilter,struct TranscoderCodecContext *pEncoderContext)
{
    
    int ret=-1;
    if (pOutput->codec_type==AVMEDIA_TYPE_VIDEO)
    {
        AVRational frameRate={1,30};
        int width=pDecoderContext->ctx->width;
        int height=pDecoderContext->ctx->height;
        enum AVPixelFormat picFormat=pDecoderContext->ctx->pix_fmt;
        
        if (pFilter) {
            
            width=av_buffersink_get_w(pFilter->sink_ctx);
            height=av_buffersink_get_h(pFilter->sink_ctx);
            picFormat= av_buffersink_get_format(pFilter->sink_ctx);
        }
        
        ret=init_video_encoder(pEncoderContext,
                               pDecoderContext->ctx->sample_aspect_ratio,
                               picFormat,
                               frameRate,
                               pOutput,
                               width,
                               height);
        
    }
    if (pOutput->codec_type==AVMEDIA_TYPE_AUDIO)
    {
        ret=init_audio_encoder(pEncoderContext, pFilter);
    }
    return ret;
}

int add_output(struct TranscodeContext* pContext, struct TranscodeOutput * pOutput)
{
    struct TranscoderCodecContext *pDecoderContext=&pContext->decoder[0];
    pContext->output[pContext->outputs++]=pOutput;
    int ret=0;
    
    if (!pOutput->passthrough)
    {
        struct TranscoderFilter* pFilter=GetFilter(pContext,pOutput,pDecoderContext);
        struct TranscoderCodecContext* pEncoderContext=&pContext->encoder[pContext->encoders];
        
        ret=config_encoder(pOutput, pDecoderContext, pFilter, pEncoderContext);
        if (ret<0) {
            return ret;
        }
        
        pOutput->encoderId=pContext->encoders++;
        LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Output %s - Added encoder %d bitrate=%d",pOutput->name,pOutput->encoderId,pOutput->bitrate*1000);
        
        struct AVCodecParameters* pCodecParams=avcodec_parameters_alloc();
        avcodec_parameters_from_context(pCodecParams,pEncoderContext->ctx);
        set_output_format(pOutput,pCodecParams);
    } else
    {
        set_output_format(pOutput,pContext->inputCodecParams);
        
    }
    
    return 0;
}


/* processing */
int encodeFrame(struct TranscodeContext *pContext,int encoderId,int outputId,AVFrame *pFrame) {
 
    struct TranscoderCodecContext* pEncoder=&pContext->encoder[encoderId];
    struct TranscodeOutput* pOutput=pContext->output[outputId];
    
    LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG, "[%s] Sending packet to encoderId %d, pts=%s key=%s",
           pOutput->name,
           encoderId,
           ts2str(pFrame->pts,true),
           (pFrame->flags & AV_PKT_FLAG_KEY)==AV_PKT_FLAG_KEY ? "I" : "P");
    
    
    int ret=0;
    
    
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
        
        LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[%s] encoded frame from encoder Id %d: pts=%s  size=%d key=%s",
               pOutput->name,
               encoderId,
               ts2str(pOutPacket->pts,true),
               pOutPacket->size,
               (pOutPacket->flags & AV_PKT_FLAG_KEY)==AV_PKT_FLAG_KEY ? "I" : "P");
        
        
        send_output_packet(pOutput,pOutPacket);
        
        av_packet_free(&pOutPacket);
    }
    return 0;
}

int sendFrameToFilter(struct TranscodeContext *pContext,int filterId, AVCodecContext* pDecoderContext, AVFrame *pFrame)
{
    
    struct TranscodeFilter *pFilter=(struct TranscodeFilter *)&pContext->filter[filterId];
    LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[0] sending frame to filter %d (%s): pts=%s",
           filterId,
           pContext->filter[filterId].config,
           ts2str(pFrame->pts,true));
    
    int ret=send_filter_frame(pFilter,pFrame);
    if (ret<0) {
        
        LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"[0] failed sending frame to filterId %d (%s): pts=%s %d (%s)",
               filterId,
               pContext->filter[filterId].config,
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
            av_frame_free(&pOutFrame);
            return ret;
        }
        
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[0] recieved video from filterId %d (%s): pts=%s, frame type=%s;width=%d;height=%d",
                   filterId,
                   pContext->filter[filterId].config,
                   ts2str(pOutFrame->pts,true),
                   pict_type_to_string(pOutFrame->pict_type),
                   pOutFrame->width,pOutFrame->height);
        }
        
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_AUDIO) {
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[0] recieved audio from filterId(%d): pts=%s, size=%d",
                   filterId,
                   ts2str(pOutFrame->pts,true),pOutFrame->nb_samples);
        }
        
        for (int outputId=0;outputId<pContext->outputs;outputId++) {
            struct TranscodeOutput *pOutput=pContext->output[outputId];
            if (pOutput->filterId==filterId){
                LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[%s] sending frame from filterId %d to encoderId %d",pOutput->name,filterId,pOutput->encoderId);
                encodeFrame(pContext,pOutput->encoderId,outputId,pOutFrame);
            }
        }
        av_frame_free(&pOutFrame);
    }
    return 0;
}

bool shouldDrop(AVFrame *pFrame)
{
    return false;
}

int OnDecodedFrame(struct TranscodeContext *pContext,AVCodecContext* pDecoderContext, AVFrame *pFrame)
{
    if (pFrame!=NULL) {
        if (pDecoderContext->codec_type==AVMEDIA_TYPE_VIDEO) {
            
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[0] decoded video: pts=%s, key=%s;pictype=%s;width=%d;height=%d",
                   pFrame->key_frame==1 ? "True" : "False",
                   ts2str(pFrame->pts,true),
                   pict_type_to_string(pFrame->pict_type),
                   pFrame->width,pFrame->height);
            
            if (shouldDrop(pFrame))
            {
                return 0;
            }
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
            LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG,"[0] sending frame directly from decoder to encoderId %d for output %s",pOutput->encoderId,pOutput->name);
            encodeFrame(pContext,pOutput->encoderId,outputId,pFrame);
        }
    }
    
    return 0;
}

int decodePacket(struct TranscodeContext *transcodingContext,const AVPacket* pkt) {
    
    int ret;
    
    
    if (pkt!=NULL) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_DEBUG, "[%d] Sending packet  to decoder pts=%s dts=%s",
               pkt->stream_index,
               ts2str(pkt->pts,true),
               ts2str(pkt->dts,true));
    }
    struct TranscoderCodecContext* pDecoder=&transcodingContext->decoder[0];
    

    ret = send_decoder_packet(pDecoder, pkt);
    if (ret < 0) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR, "[%d] Error sending a packet for decoding %d (%s)",pkt->stream_index,ret,av_err2str(ret));
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
            LOGGER(CATEGORY_DEFAULT,AV_LOG_ERROR,"[%d] Error during decoding %d (%s)",pkt->stream_index,ret,av_err2str(ret));
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


/* shutting down */

int close_transcoding_context(struct TranscodeContext *pContext) {
    
    convert_packet(pContext,NULL);

    return 0;
}

//
//  filter.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 01/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "filter.h"
#include "logger.h"

int init_filter(struct TranscoderFilter *pFilter,struct AVStream *pInputStream, AVCodecContext *dec_ctx,const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = pInputStream->time_base;
    
    pFilter->filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !pFilter->filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
             time_base.num, time_base.den,
             dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    
    
    ret = avfilter_graph_create_filter(&pFilter->src_ctx, buffersrc, "in",
                                       args, NULL, pFilter->filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }
    
    ret = avfilter_graph_create_filter(&pFilter->sink_ctx, buffersink, "out",
                                       NULL, NULL, pFilter->filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }
    
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = pFilter->src_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;
    
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = pFilter->sink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;
    
    if ((ret = avfilter_graph_parse_ptr(pFilter->filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;
    
    if ((ret = avfilter_graph_config(pFilter->filter_graph, NULL)) < 0)
        goto end;
    
end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    
    return ret;
}

int send_filter_frame(struct TranscoderFilter *pFilter,struct AVFrame* pInFrame)
{
    int ret=0;
    ret = av_buffersrc_write_frame(pFilter->src_ctx, pInFrame);
    return ret;
}


int receive_filter_frame(struct TranscoderFilter *pFilter,struct AVFrame* pOutFrame)
{
    int ret=0;
    ret = av_buffersink_get_frame(pFilter->sink_ctx, pOutFrame);
    if (ret<0) {
        logger(0,"error %d",ret);
    }
    return ret;
}



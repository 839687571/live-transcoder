//
//  filter.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 01/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "filter.h"
#include "logger.h"

int init_filter(struct TranscoderFilter *pFilter, AVCodecContext *dec_ctx,const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc=NULL;
    const AVFilter *buffersink=NULL;
    
    
    if (dec_ctx->codec_type==AVMEDIA_TYPE_VIDEO) {
        buffersrc  = avfilter_get_by_name("buffer");
        buffersink = avfilter_get_by_name("buffersink");
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
                 dec_ctx->time_base.num, dec_ctx->time_base.den,
                 dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    }
    if (dec_ctx->codec_type==AVMEDIA_TYPE_AUDIO) {
        buffersrc  = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        snprintf(args, sizeof args,
                 "sample_rate=%d:sample_fmt=%d:channel_layout=0x%"PRIx64":channels=%d:"
                 "time_base=%d/%d",
                 dec_ctx->sample_rate, dec_ctx->sample_fmt, dec_ctx->channel_layout,
                 dec_ctx->channels, dec_ctx->time_base.num, dec_ctx->time_base.den);
    }
    
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    
    pFilter->config=strdup(filters_descr);
    
    pFilter->filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !pFilter->filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    
    
    ret = avfilter_graph_create_filter(&pFilter->src_ctx, buffersrc, "in",
                                       args, NULL, pFilter->filter_graph);
    if (ret < 0) {
        LOGGER("FILTER",AV_LOG_ERROR, "Cannot create buffer source %d",ret)
        goto end;
    }
    
    ret = avfilter_graph_create_filter(&pFilter->sink_ctx, buffersink, "out",
                                       NULL, NULL, pFilter->filter_graph);
    if (ret < 0) {
        LOGGER("FILTER", AV_LOG_ERROR, "Cannot create buffer sink %d",ret)
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
    return ret;
}



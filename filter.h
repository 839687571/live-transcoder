//
//  filter.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 01/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef filter_h
#define filter_h

#include "core.h"

struct TranscoderFilter
{
    char* config;
    AVFilterGraph* filter_graph;
    AVFilterContext *sink_ctx;
    AVFilterContext *src_ctx;
};

int init_filter(struct TranscoderFilter *pFilter, AVCodecContext *dec_ctx,const char *filters_descr);
int send_filter_frame(struct TranscoderFilter *pFilter,struct AVFrame* pInFrame);
int receive_filter_frame(struct TranscoderFilter *pFilter,struct AVFrame* pOutFrame);
int close_filter(struct TranscoderFilter *pFilter);

#endif /* filter_h */

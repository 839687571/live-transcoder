//
//  TranscodePipeline.hpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef TranscodePipeline_hpp
#define TranscodePipeline_hpp


#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include "output.h"
#include "codec.h"
#include "filter.h"

#define MAX_INPUTS 10
#define MAX_OUTPUTS 10


struct TranscodeContext {
    
    struct AVCodecParameters* inputCodecParams;
    
    int decoders;
    struct TranscoderCodecContext decoder[MAX_INPUTS];
    
    
    int outputs;
    struct TranscodeOutput* output[MAX_OUTPUTS];
    
    int encoders;
    struct TranscoderCodecContext encoder[MAX_OUTPUTS];

    
    int filters;
    struct TranscoderFilter filter[10];
    
};


/*
 0
 1
 */

int init_transcoding_context(struct TranscodeContext *ctx,struct AVCodecParameters* codecParams,AVRational framerate);
int convert_packet(struct TranscodeContext *pContext, struct AVPacket* packet);
int close_transcoding_context(struct TranscodeContext *ctx);
int transcoding_context_to_json(struct TranscodeContext *ctx,char* buf);


#endif /* TranscodePipeline_hpp */

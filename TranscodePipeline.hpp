//
//  TranscodePipeline.hpp
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef TranscodePipeline_hpp
#define TranscodePipeline_hpp

#define __STDC_CONSTANT_MACROS
extern "C" {
    #include <libavutil/timestamp.h>
    #include <libavformat/avformat.h>
    #include <libavfilter/buffersink.h>
    #include <libavfilter/buffersrc.h>
}

#include "input.hpp"
#include "output.hpp"
#include "filter.hpp"

#define MAX_INPUTS 10
#define MAX_OUTPUTS 10


struct TranscodeContext {
    
    struct InputContext* pInputContext;
    
    
    int inputs;
    AVCodec* input_codecs[MAX_INPUTS];
    AVCodecContext* input_codec_contexts[MAX_INPUTS];
    
    
    int outputs;
    struct TranscodeOutput* output[MAX_OUTPUTS];
    
    int encoders;
    AVCodec* output_codec[MAX_OUTPUTS];
    AVCodecContext* output_codec_contexts[MAX_OUTPUTS];
    
    
    int filters;
    struct TranscoderFilter filter[10];
    
};


/*
 0
 1
 */

int init_transcoding_context(struct TranscodeContext *ctx,struct InputContext* inputContext);

#endif /* TranscodePipeline_hpp */

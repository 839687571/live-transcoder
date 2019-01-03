//
//  input.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef input_h
#define input_h

#include <stdio.h>
#include "logger.hpp"
extern "C" {
    #include <libavformat/avformat.h>
}

struct InputContext;
struct TranscodeContext;

typedef int (*OnPacketCallBack)(struct TranscodeContext *transcodingCtx,struct InputContext *inputCtx,AVStream* pStream,AVPacket* packet);

struct InputContext {
    
    struct TranscodeContext* pTranscodingContext;
    char *pSourceFileName;
    AVFormatContext *ifmt_ctx;
    OnPacketCallBack pOnPacketCB;
    
};

int init_input(struct InputContext *ctx,const char* fileName);
void process(struct InputContext *ctx);

#endif /* input_h */

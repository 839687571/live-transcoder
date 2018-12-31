//
//  input.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "input.h"

int init_input(struct InputContext *ctx,const char* fileName)
{
    ctx->pOnPacketCB=NULL;
    ctx->pTranscodingContext=NULL;
    ctx->pSourceFileName=strdup(fileName);
    int ret = avformat_open_input(&ctx->ifmt_ctx, ctx->pSourceFileName, NULL, NULL);
    
    if (ret < 0) {
        char buff[256];
        av_strerror(ret, buff, 256);
        logger(AV_LOG_DEBUG,"len: Unable to open input %s %s(%d)",ctx->pSourceFileName,buff,ret);
        return ret;
        
    }
    ret = avformat_find_stream_info(ctx->ifmt_ctx, NULL);
    if (ret < 0) {
        logger(AV_LOG_DEBUG,"segmenter: Unable to find any input streams");
    }
    return 0;
    
}

void process(struct InputContext *ctx)
{
    AVPacket packet;
    int ret;
    
    
    
    AVFormatContext* ifmt_ctx=ctx->ifmt_ctx;
    av_init_packet(&packet);
    
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        if (ctx->pOnPacketCB) {
            ctx->pOnPacketCB(ctx->pTranscodingContext,ctx,ifmt_ctx->streams[packet.stream_index],&packet);
        }
        
    }
}

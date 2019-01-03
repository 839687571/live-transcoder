//
//  TranscoderEncoder.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 03/01/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef TranscoderEncoder_h
#define TranscoderEncoder_h

#include <stdio.h>

#include <libavformat/avformat.h>

struct TranscoderCodecContext
{
    AVCodec* codec;
    AVCodecContext* ctx;
};

int init_decoder(struct TranscoderCodecContext * pContext,AVStream *pInputStream);
int encode_frame(struct TranscoderCodecContext *encoder,const AVFrame* pFrame);

int send_decoder_packet(struct TranscoderCodecContext *decoder,const AVPacket* pkt);
int receive_decoder_frame(struct TranscoderCodecContext *decoder,AVFrame *pFrame);

    
#endif /* TranscoderEncoder_h */

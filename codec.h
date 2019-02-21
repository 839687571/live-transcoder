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
#include "filter.h"
#include "output.h"

struct TranscoderCodecContext
{
    AVCodec* codec;
    AVCodecContext* ctx;
};

int init_decoder(struct TranscoderCodecContext * pContext,AVCodecParameters *pCodecParams);

int init_video_encoder(struct TranscoderCodecContext * pContext,
                       AVRational inputAspectRatio,
                       enum AVPixelFormat inputPixelFormat,
                       AVRational inputTimeBase,
                       const struct TranscodeOutput* pOutput,
                       int width,int height);

int init_audio_encoder(struct TranscoderCodecContext * pContext,struct TranscoderFilter* pFilter);


int send_encode_frame(struct TranscoderCodecContext *encoder,const AVFrame* pFrame);
int receive_encoder_packet(struct TranscoderCodecContext *encoder,AVPacket* pkt);

int send_decoder_packet(struct TranscoderCodecContext *decoder,const AVPacket* pkt);
int receive_decoder_frame(struct TranscoderCodecContext *decoder,AVFrame *pFrame);

    
#endif /* TranscoderEncoder_h */

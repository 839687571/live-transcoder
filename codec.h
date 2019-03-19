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
    AVBufferRef *hw_device_ctx,*hw_frames_ctx;
    AVCodec* codec;
    AVCodecContext* ctx;
    int64_t inPts,outPts;
    bool nvidiaAccelerated;
};

int init_decoder(struct TranscoderCodecContext * pContext,AVCodecParameters *pCodecParams);

int close_codec(struct TranscoderCodecContext * pContext);

int init_video_encoder(struct TranscoderCodecContext * pContext,
                       AVRational inputAspectRatio,
                       enum AVPixelFormat inputPixelFormat,
                       AVRational inputFrameRate,
                       struct AVBufferRef* hw_frames_ctx,
                       const struct TranscodeOutput* pOutput,
                       int width,int height);

int init_audio_encoder(struct TranscoderCodecContext * pContext,struct TranscoderFilter* pFilter);


int send_encode_frame(struct TranscoderCodecContext *encoder,const AVFrame* pFrame);
int receive_encoder_packet(struct TranscoderCodecContext *encoder,AVPacket* pkt);

int send_decoder_packet(struct TranscoderCodecContext *decoder,const AVPacket* pkt);
int receive_decoder_frame(struct TranscoderCodecContext *decoder,AVFrame *pFrame);

int reset_decoder(struct TranscoderCodecContext *decoder);

inline int64_t get_latency(struct TranscoderCodecContext *codec) { return llabs(codec->outPts-codec->inPts);}
#endif /* TranscoderEncoder_h */

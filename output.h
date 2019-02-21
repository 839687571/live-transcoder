//
//  output.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef output_h
#define output_h

#include <stdio.h>
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <sys/time.h>
#include "FramesStats.h"

enum TranscodeOutputType
{
    TranscodeOutputType_Video,
    TranscodeOutputType_Audio
};


struct TranscodeOutput
{
    char* name;
    enum AVMediaType codec_type;
    bool passthrough;
    int bitrate;
    struct VideoParams
    {
        int width,height;
        int frameRate;
        char* profile;
        char* level;
        float fps;
    } videoParams;
    
    struct
    {
        int samplingRate, channels;
    } audioParams;
    
    int filterId;
    int encoderId;
    
    struct FramesStats stats;
    
    
    AVFormatContext *oc;
    AVBSFContext* bsf;
};


int init_Transcode_output(struct TranscodeOutput* pOutput) ;
int close_Transcode_output(struct TranscodeOutput* pOutput) ;

int set_output_format(struct TranscodeOutput *pOutput,struct AVCodecParameters* output) ;
int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output) ;
    
#endif /* output_h */



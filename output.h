//
//  output.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef output_h
#define output_h

#include "core.h"
#include "FramesStats.h"
#include "json_parser.h"

enum TranscodeOutputType
{
    TranscodeOutputType_Video,
    TranscodeOutputType_Audio
};


struct TranscodeOutput
{
    char* name;
    char* codec;
    enum AVMediaType codec_type;
    bool passthrough;
    int bitrate;
    struct VideoParams
    {
        int width,height;
        int frameRate;
        char* profile;
        char* level;
        char* preset;
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
int init_Transcode_output_from_json(struct TranscodeOutput* ,const json_value_t* json);
int close_Transcode_output(struct TranscodeOutput* pOutput) ;

int set_output_format(struct TranscodeOutput *pOutput,struct AVCodecParameters* output) ;
int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output) ;

#endif /* output_h */



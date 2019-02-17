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
    int width,height,vid_bitrate;
    float fps;
    int samplingRate, channels, audio_bitrate;
};

int init_Transcode_output(struct TranscodeOutput* pOutput) ;

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output) ;
    
#endif /* output_h */



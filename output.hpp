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

extern "C" {
#include <libavformat/avformat.h>
}

struct TranscodeOutput
{
    bool vid_passthrough,aud_passthrough;
    int width,height,vid_bitrate;
    float fps;
    int samplingRate, channels, audio_bitrate;
};

int init_Transcode_output(struct TranscodeOutput* pOutput) ;

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output) ;
    
#endif /* output_h */



//
//  output.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "output.hpp"


int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->width=pOutput->height=pOutput->vid_bitrate=-1;
    pOutput->fps=-1;
    pOutput->samplingRate=pOutput->channels=pOutput->audio_bitrate=-1;
    pOutput->vid_passthrough=pOutput->aud_passthrough=true;
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output)
{
    return 0;
}

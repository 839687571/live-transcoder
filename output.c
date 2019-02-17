//
//  output.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include "output.h"

#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->width=pOutput->height=pOutput->vid_bitrate=-1;
    pOutput->fps=-1;
    pOutput->samplingRate=pOutput->channels=pOutput->audio_bitrate=-1;
    pOutput->passthrough=true;
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output)
{
    
    logger(AV_LOG_ERROR,"output (%s) got data: pts=%s , size=%d, flags=%d",pOutput->name,
           av_ts2str(output->pts),
           output->size,
           output->flags);
    
    return 0;
}

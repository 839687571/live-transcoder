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
#include "logger.h"

int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->width=pOutput->height=pOutput->vid_bitrate=-1;
    pOutput->fps=-1;
    pOutput->samplingRate=pOutput->channels=pOutput->audio_bitrate=-1;
    pOutput->passthrough=true;
    pOutput->filter=-1;
    pOutput->encoder=-1;
    
    InitFrameStats(&pOutput->stats);
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output)
{
    AddFrameToStats(&pOutput->stats,output->pts,output->size);
    logger(CATEGORY_OUTPUT, AV_LOG_ERROR,"output (%s) got data: pts=%s , size=%d, flags=%d totalFrames=%ld, bitrate %.lf",pOutput->name,
           av_ts2str(output->pts),
           output->size,
           output->flags,
           pOutput->stats.totalFrames,
           GetFrameStatsAvg(&pOutput->stats));
    
    return 0;
}


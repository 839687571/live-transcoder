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

static  AVRational standard_timebase = {1,1000};


int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->name="";
    pOutput->bitrate=-1;
    pOutput->passthrough=true;
    pOutput->filterId=-1;
    pOutput->encoderId=-1;
    
    InitFrameStats(&pOutput->stats);
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* output)
{
    AddFrameToStats(&pOutput->stats,output->dts,output->size);
    logger(CATEGORY_OUTPUT, AV_LOG_DEBUG,"output (%s) got data: pts=%s (%s), size=%d, flags=%d totalFrames=%ld, bitrate %.lf",pOutput->name,
           av_ts2str(output->dts),
           av_ts2timestr(output->dts, &standard_timebase),
           output->size,
           output->flags,
           pOutput->stats.totalFrames,
           GetFrameStatsAvg(&pOutput->stats));
    
    return 0;
}


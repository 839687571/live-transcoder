//
//  Stats.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "FramesStats.h"
#include "logger.h"


void InitFrameStats(struct FramesStats* pStats)
{
    pStats->totalFrames=0;
}
void AddFrameToStats(struct FramesStats* pStats,uint64_t frame_time,int size)
{
    pStats->frames[pStats->totalFrames % HISTORY_SIZE] = size;
    pStats->times[pStats->totalFrames % HISTORY_SIZE] = frame_time;
    pStats->totalFrames++;
}

double GetFrameStatsAvg(struct FramesStats* pStats)
{
    return 0;
    if (pStats->totalFrames>1) {
        uint64_t total=0;
        uint64_t now = pStats->times[(pStats->totalFrames-1) % HISTORY_SIZE];

        for (uint64_t runner=pStats->totalFrames-1;runner>=0;runner--) {
            uint64_t timePassed=now-pStats->times[runner % HISTORY_SIZE];
            if (runner==0 || timePassed>5000) {
                return ((double)(total*8))/timePassed;
            }
            total+=pStats->frames[runner % HISTORY_SIZE];
        }
    }
    return 0;
}

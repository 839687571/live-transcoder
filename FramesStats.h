//
//  Stats.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef Stats_h
#define Stats_h

#include <stdio.h>

#define HISTORY_SIZE 1000
struct FramesStats
{
    uint64_t totalFrames;
    
    u_int frames[HISTORY_SIZE];
    uint64_t times[HISTORY_SIZE];
    
};

void InitFrameStats(struct FramesStats* pStats);
void AddFrameToStats(struct FramesStats* pStats,uint64_t frame_time,int size);
double GetFrameStatsAvg(struct FramesStats* pStats);

#endif /* Stats_h */

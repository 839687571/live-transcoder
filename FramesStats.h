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

#define HISTORY_DURATION 10LL //10 seconds
#define HISTORY_SIZE 700LL

struct FramesStatsHistory
{
    u_int frameSize;
    uint64_t pts;
    uint64_t clock;
};

struct FramesStats
{
    uint64_t totalFrames;
    uint64_t head,tail;
    struct FramesStatsHistory history[HISTORY_SIZE];
    
    int64_t totalBitrateInWindow;
    
};

void InitFrameStats(struct FramesStats* pStats);
void AddFrameToStats(struct FramesStats* pStats,uint64_t pts,int size);
void GetFrameStatsAvg(struct FramesStats* pStats,int* bitRate,double *fps,double*rate);

#endif /* Stats_h */

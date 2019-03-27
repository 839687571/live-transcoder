//
//  Stats.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "FramesStats.h"
#include "logger.h"
#include "utils.h"
#include "json_parser.h"


void InitFrameStats(struct FramesStats* pStats,AVRational basetime)
{
    pStats->totalFrames=0;
    pStats->head=-1;
    pStats->tail=-1;
    pStats->totalWindowSizeInBytes=0;
    pStats->basetime=basetime;
    pStats->currentBitRate=0;
    pStats->currentRate=0;
    pStats->currentFrameRate=0;
}

void drain(struct FramesStats* pStats,uint64_t clock)
{
    while (pStats->tail<pStats->head) {
        struct FramesStatsHistory  *pTail=( struct FramesStatsHistory  *)&pStats->history[ pStats->tail  % HISTORY_SIZE];
        uint64_t timePassed=clock - pTail->pts;
        if (timePassed<HISTORY_DURATION*90000) {
            break;
        }
        pStats->totalWindowSizeInBytes-=pTail->frameSize;
        pStats->tail++;
    }
}

struct FramesStatsHistory*   getIndex(struct FramesStats* pStats,int64_t index) {
    return ( struct FramesStatsHistory  *)&(pStats->history[index % HISTORY_SIZE]);
}


void calculate_stats(struct FramesStats* pStats)
{
    if (pStats->head!=-1 && pStats->head!=pStats->tail) {
        struct FramesStatsHistory  *pHead=getIndex(pStats,pStats->head);
        struct FramesStatsHistory  *pTail=getIndex(pStats,pStats->tail);
        
        double timePassedInSec= (pHead->clock - pTail->clock )  / 1000000.0;
        double ptsPassedInSec= (pHead->pts - pTail->pts )  / 90000.0;
        
        pStats->ptsPassed=(pHead->pts - pStats->firstPts);
        
        int64_t frames=(pStats->head - pStats->tail + 1);
        
        if (ptsPassedInSec>0 && timePassedInSec>0) {
            double dbitRate= (double)(pStats->totalWindowSizeInBytes*8)/ptsPassedInSec;
            pStats->currentBitRate=(int)dbitRate;
            pStats->currentFrameRate=frames/timePassedInSec;
            pStats->currentRate=ptsPassedInSec/timePassedInSec;
        }
    }
}


void AddFrameToStats(struct FramesStats* pStats,uint64_t pts,int frameSize)
{
    pStats->head++;
    if (pStats->head==0){
        pStats->tail=0;
        pStats->firstPts=pts;
    }
    struct FramesStatsHistory  *pHead=getIndex(pStats,pStats->head);
    pHead->frameSize=frameSize;
    pHead->pts=pts;
    pHead->clock=getTime64();
    
    pStats->totalFrames++;
    pStats->totalWindowSizeInBytes+=frameSize;
    
    drain(pStats,pHead->pts);
    calculate_stats(pStats);
}


int stats_to_json(struct FramesStats *pStats,char* buf)
{
    JSON_SERIALIZE_INIT(buf)
    JSON_SERIALIZE_INT64("totalSamples",pStats->totalFrames)
    JSON_SERIALIZE_INT("bitrate",pStats->currentBitRate)
    JSON_SERIALIZE_DOUBLE("fps",pStats->currentFrameRate)
    JSON_SERIALIZE_DOUBLE("rate",pStats->currentRate)
    JSON_SERIALIZE_END()
    return n;
}


void log_frame_stats(const char* category,int level,struct FramesStats *stats,const char*prefix)
{
    LOGGER(category,level,"[%s] Stats: total frames: %lld total time: %s, bitrate %.2lf Kbit/s fps=%.2lf rate=x%.2lf",
           prefix,
           stats->totalFrames,
           ts2str(stats->ptsPassed, true),
           ((double)stats->currentBitRate)/(1000.0),
           stats->currentFrameRate,
           stats->currentRate)
}

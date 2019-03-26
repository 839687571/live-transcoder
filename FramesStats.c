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
}

void GetFrameStatsAvg(struct FramesStats* pStats,int* bitRate,double *fps,double *rate)
{
    *bitRate=0;
    *fps=0;
    *rate=0;
    if (pStats->head!=-1 && pStats->head!=pStats->tail) {
        struct FramesStatsHistory  *pHead=getIndex(pStats,pStats->head);
        struct FramesStatsHistory  *pTail=getIndex(pStats,pStats->tail);

        double timePassedInSec= (pHead->clock - pTail->clock )  / 1000000.0;
        double ptsPassedInSec= (pHead->pts - pTail->pts )  / 90000.0;

        int64_t frames=(pStats->head - pStats->tail + 1);

        if (ptsPassedInSec>0 && timePassedInSec>0) {
            double dbitRate= (double)(pStats->totalWindowSizeInBytes*8)/ptsPassedInSec;
            *bitRate=(int)dbitRate;
            *fps=frames/timePassedInSec;
            *rate=ptsPassedInSec/timePassedInSec;
        }
    }
}

int stats_to_json(struct FramesStats *pStats,char* buf)
{
    int bitRate;
    double fps;
    double rate;
    GetFrameStatsAvg(pStats,&bitRate,&fps,&rate);
    
    JSON_SERIALIZE_INIT(buf)
    JSON_SERIALIZE_INT64("totalSamples",pStats->totalFrames)
    JSON_SERIALIZE_INT("bitrate",bitRate)
    JSON_SERIALIZE_DOUBLE("fps",fps)
    JSON_SERIALIZE_DOUBLE("rate",rate)
    JSON_SERIALIZE_END()
    return n;
}


void log_frame_stats(const char* category,int level,struct FramesStats *stats,const char*prefix)
{
    int avgBitrate;
    double fps,rate;
    GetFrameStatsAvg(stats,&avgBitrate,&fps,&rate);
    LOGGER(category,level,"[%s] Stats: total frames: %ld bitrate %.2lf Kbit/s fps=%.2lf rate=x%.2lf",
           prefix,
           stats->totalFrames,
           ((double)avgBitrate)/(1000.0),
           fps,
           rate)
}

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
#include "libavutil/intreadwrite.h"

static  AVRational standard_timebase = {1,1000};



int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->name="";
    pOutput->bitrate=-1;
    pOutput->passthrough=true;
    pOutput->filterId=-1;
    pOutput->encoderId=-1;
    pOutput->pOutputFile=NULL;
    pOutput->videoParams.width=pOutput->videoParams.height=pOutput->videoParams.fps=-1;
    pOutput->audioParams.samplingRate=pOutput->audioParams.channels=-1;
    
    InitFrameStats(&pOutput->stats);
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* packet)
{
  
    AddFrameToStats(&pOutput->stats,packet->dts,packet->size);
    
    LOGGER(CATEGORY_OUTPUT,AV_LOG_DEBUG,"output (%s) got data: pts=%s; dts=%s, size=%d, flags=%d totalFrames=%ld, bitrate %.lf",
           pOutput->name,
           ts2str(packet->pts,true),
           ts2str(packet->dts,true),
           packet->size,
           packet->flags,
           pOutput->stats.totalFrames,
           GetFrameStatsAvg(&pOutput->stats))
    
    
    if (pOutput->pOutputFile!=NULL && packet->data) {
        
        if (AV_RB32(packet->data) == 0x00000001 ||
            AV_RB24(packet->data) == 0x000001) {
            fwrite(packet->data,1,packet->size,pOutput->pOutputFile);
            return 0;
        }
        
        AVPacket in_pkt = { 0 };
        in_pkt.data = (uint8_t *)packet->data;
        in_pkt.size = packet->size;
        av_bsf_send_packet(pOutput->bsf,&in_pkt);
        
        int ret=0;
        while ((ret = av_bsf_receive_packet(pOutput->bsf, &in_pkt)) == 0)
        {
        
            if (ret == AVERROR(EAGAIN))
                continue;
            
            fwrite(in_pkt.data,1,in_pkt.size,pOutput->pOutputFile);
        }
        
        
        //LOGGER(CATEGORY_OUTPUT,AV_LOG_FATAL,"(%s) not mp4 bit format",pOutput->name)
    }
    return 0;
}

int set_output_format(struct TranscodeOutput *pOutput,struct AVCodecParameters* codecParams)
{
    if (pOutput->pOutputFile==NULL) {
        char* tmp[1000];
        sprintf(tmp,"/Users/guyjacubovski/dev/live-transcoder/output_%s.h264", pOutput->name);
        pOutput->pOutputFile= fopen(tmp,"wb+");  // r for read, b for binary
        
        const AVBitStreamFilter *bsf = av_bsf_get_by_name("h264_mp4toannexb");
        int ret = av_bsf_alloc(bsf, &pOutput->bsf);
        if (ret < 0)
            return ret;
        
        ret = avcodec_parameters_copy(pOutput->bsf->par_in, codecParams);
        if (ret < 0)
            return ret;
        
        
        ret = av_bsf_init(pOutput->bsf);
        if (ret < 0) {
            return ret;
        }
        
        

    }
    if (codecParams && codecParams->extradata!=NULL && codecParams->extradata_size>0) {
        fwrite(codecParams->extradata,1,codecParams->extradata_size,pOutput->pOutputFile);
    }
    return 0;
}



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
#include "libavutil/intreadwrite.h"
#include "utils.h"
#include "logger.h"
#include "config.h"


int init_Transcode_output(struct TranscodeOutput* pOutput)  {
    pOutput->name="";
    pOutput->bitrate=-1;
    pOutput->codec_type=AVMEDIA_TYPE_UNKNOWN;
    pOutput->passthrough=true;
    pOutput->filterId=-1;
    pOutput->encoderId=-1;
    pOutput->oc=NULL;
    pOutput->videoParams.width=pOutput->videoParams.height=pOutput->videoParams.fps=-1;
    pOutput->videoParams.frameRate=-1;
    pOutput->videoParams.level="";
    pOutput->videoParams.profile="";
    pOutput->audioParams.samplingRate=pOutput->audioParams.channels=-1;
    
    InitFrameStats(&pOutput->stats);
    return 0;
}

int print_output(struct TranscodeOutput* pOutput) {
    
    if ( pOutput->passthrough) {
        LOGGER(CATEGORY_OUTPUT,AV_LOG_INFO,"(%s) output configuration: mode: passthrough",pOutput->name);
        return 0;
    }
    if (pOutput->codec_type==AVMEDIA_TYPE_VIDEO) {
        LOGGER(CATEGORY_OUTPUT,AV_LOG_INFO,"(%s) output configuration: mode: transcode bitrate: %d Kbit/s  resolution: %dx%d  fps: %.2f profile: %s preset: %s",
               pOutput->name,
               pOutput->bitrate,
               pOutput->videoParams.width,
               pOutput->videoParams.height,
               pOutput->videoParams.fps,
               pOutput->videoParams.profile,
               pOutput->videoParams.preset
               )
    }
    if (pOutput->codec_type==AVMEDIA_TYPE_AUDIO) {
        LOGGER(CATEGORY_OUTPUT,AV_LOG_INFO,"(%s) output configuration: mode: transcode bitrate: %d Kbit/s  %d channels, %d Hz",
               pOutput->name,
               pOutput->bitrate,
               pOutput->audioParams.channels,
               pOutput->audioParams.samplingRate
               )
    }
    
    return 0;
}

int init_Transcode_output_from_json(struct TranscodeOutput* pOutput,const json_value_t* json)
{
    init_Transcode_output(pOutput);

    
    json_get_string(json,"name","",&(pOutput->name));
    json_get_int(json,"bitrate",-1,&(pOutput->bitrate));
    json_get_bool(json,"passthrough",true,&(pOutput->passthrough));
    json_get_string(json,"name","",&(pOutput->name));
    json_get_string(json,"codec","",&pOutput->codec);
    const json_value_t* pVideoParams,*pAudioParams;
    if (JSON_OK==json_get(json,"videoParams",&pVideoParams)) {
        pOutput->codec_type=AVMEDIA_TYPE_VIDEO;
        pOutput->videoParams.width=-2;
        json_get_int(pVideoParams,"height",-1,&pOutput->videoParams.height);
        json_get_string(pVideoParams,"profile","",&pOutput->videoParams.profile);
        json_get_string(pVideoParams,"preset","veryfast",&pOutput->videoParams.preset);
    }
    if (JSON_OK==json_get(json,"audioParams",&pAudioParams)) {
        pOutput->codec_type=AVMEDIA_TYPE_AUDIO;
        json_get_int(pAudioParams,"channels",2,&pOutput->audioParams.channels);
        json_get_int(pAudioParams,"samplingRate",48000,&pOutput->audioParams.samplingRate);
    }

    print_output(pOutput);
    return 0;
}

int send_output_packet(struct TranscodeOutput *pOutput,struct AVPacket* packet)
{
    if (packet==NULL){
        return 0;
    }
    AddFrameToStats(&pOutput->stats,packet->dts,packet->size);
    int avgBitrate;
    double fps,rate;
    GetFrameStatsAvg(&pOutput->stats,&avgBitrate,&fps,&rate);
    LOGGER(CATEGORY_OUTPUT,AV_LOG_DEBUG,"output (%s) got data: pts=%s; dts=%s clock=%s, size=%d, flags=%d bitrate %.2lf Kbit/s fps=%.2lf rate=%.2lf",
           pOutput->name,
           ts2str(packet->pts,true),
           ts2str(packet->dts,true),
           ts2str(packet->pos,true),
           packet->size,
           packet->flags,
           ((double)avgBitrate)/(1000.0),
           fps,
           rate)
    
    if (pOutput->oc) {
        AVPacket *cpPacket=av_packet_clone(packet);
        int ret=av_write_frame(pOutput->oc, cpPacket);
    
        if (ret<0) {
            
            LOGGER(CATEGORY_OUTPUT,AV_LOG_FATAL,"(%s) cannot save frame  %d (%s)",pOutput->name,ret,av_err2str(ret))
        }
        av_write_frame(pOutput->oc, NULL);
        
        av_packet_unref(cpPacket);
    }
    /*
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
    }*/
    return 0;
}

int set_output_format(struct TranscodeOutput *pOutput,struct AVCodecParameters* codecParams)
{
    
    bool saveFile;
    json_get_bool(GetConfig(),"debug.saveFile",false,&saveFile);
    
    if (saveFile && pOutput->oc==NULL) {
        char* fileNamePattern;
        char filename[1000];
        json_get_string(GetConfig(),"debug.outputFileNamePattern","output_%s.mp4",&fileNamePattern);
        sprintf(filename,fileNamePattern, pOutput->name);
        //pOutput->pOutputFile= fopen(filename,"wb+");  // r for read, b for binary
        
        if (codecParams->codec_type==AVMEDIA_TYPE_VIDEO)
        {
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

        /* allocate the output media context */
        avformat_alloc_output_context2(&pOutput->oc, NULL, NULL, filename);
        if (!pOutput->oc) {
            LOGGER(CATEGORY_OUTPUT,AV_LOG_FATAL,"(%s) cannot create filename %s",pOutput->name,filename)
            return -1;
        }
        

        
        AVStream *st = avformat_new_stream(pOutput->oc, NULL);

        st->id=0;
        
        
        
        avcodec_parameters_copy(st->codecpar,codecParams);
        
        int ret = avio_open(&pOutput->oc->pb, filename, AVIO_FLAG_WRITE);
        if (ret<0) {
            
            LOGGER(CATEGORY_OUTPUT,AV_LOG_FATAL,"(%s) cannot create filename %s",pOutput->name,filename)
            return ret;
        }
        AVDictionary* opts = NULL;

        av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov", 0);

        ret = avformat_write_header(pOutput->oc, &opts);
        if (ret<0) {
            
            LOGGER(CATEGORY_OUTPUT,AV_LOG_FATAL,"(%s) cannot create filename %s - %d (%s)",pOutput->name,filename,ret,av_err2str(ret))
        }

    }
    return 0;
}



int close_Transcode_output(struct TranscodeOutput* pOutput)
{
    if (pOutput->oc==NULL) {
        int ret = av_write_trailer(pOutput->oc);
    
        avio_closep(&pOutput->oc->pb);
        /* free the stream */
        avformat_free_context(pOutput->oc);
        return ret;
    }
    return 0;
}

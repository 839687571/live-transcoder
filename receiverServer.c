//
//  listener.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "receiverServer.h"
#include "utils.h"
#include "logger.h"
#include <pthread.h>
#include "config.h"
#include "KMP.h"
#include "TranscodePipeline.h"


int init_outputs(struct ReceiverServer *server,struct TranscodeContext* pContext,json_value_t* json)
{
    const json_value_t* outputsJson;
    json_get(json,"outputs",&outputsJson);
    
    for (int i=0;i<json_get_array_count(outputsJson);i++)
    {
        json_value_t outputJson;
        json_get_array_index(outputsJson,i,&outputJson);
        
        bool enabled=true;
        json_get_bool(&outputJson,"enabled",true,&enabled);
        if (!enabled) {
            char* name;
            json_get_string(&outputJson,"name","",&name);
            LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Skipping output %s since it's disabled",name);
            continue;
        }
        struct TranscodeOutput *pOutput=&server->outputs[server->totalOutputs];
        init_Transcode_output_from_json(pOutput,&outputJson);
        
        add_output(pContext,pOutput);
        server->totalOutputs++;
    }
    return 0;
}



void* listenerThread(void *vargp)
{
    
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"listenerThread");
    
    struct ReceiverServer *server=(struct ReceiverServer *)vargp;
    struct TranscodeContext *transcodeContext = server->transcodeContext;
    
    json_value_t* config=GetConfig();
    
    
    if (KMP_listen(&server->kmpServer,server->port)<0) {
        exit (-1);
        return NULL;
    }
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Waiting for accept");
    pthread_cond_signal(&server->cond);

    
    struct KalturaMediaProtocolContext kmpClient;

    if (KMP_accept(&server->kmpServer,&kmpClient)<0) {
        return NULL;
    }
    

    
    AVRational frameRate;
    
    AVCodecParameters* params=avcodec_parameters_alloc();
    if (KMP_read_mediaInfo(&kmpClient,params,&frameRate)<0) {
        LOGGER0(CATEGORY_RECEIVER,AV_LOG_FATAL,"Invalid mediainfo");
        exit (-1);
    }

    if (transcodeContext!=NULL) {
        init_transcoding_context(transcodeContext,params,frameRate);
        init_outputs(server,transcodeContext,config);
    }
    
    
    InitFrameStats(&server->listnerStats,standard_timebase);
    
    AVPacket packet;
    
    while (true) {
        
        if (KMP_readPacket(&kmpClient,&packet)<=0) {
            break;
        }
        
        AddFrameToStats(&server->listnerStats,packet.pts,packet.size);

        log_frame_stats(CATEGORY_RECEIVER,AV_LOG_DEBUG,&server->listnerStats,"0");
        LOGGER(CATEGORY_RECEIVER,AV_LOG_DEBUG,"[0] received packet %s",getPacketDesc(&packet));

        packet.pos=getClock64();
        
        if (transcodeContext!=NULL)
        {
            convert_packet(transcodeContext,&packet);
        }
        av_packet_unref(&packet);
        
    }
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Destorying receive thread");

    
    if (transcodeContext!=NULL)
    {
        close_transcoding_context(transcodeContext);
        for (int i=0;i<server->totalOutputs;i++){
            LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Closing output %s",server->outputs[i].name);
            close_Transcode_output(&server->outputs[i]);
        }
    }
    
    avcodec_parameters_free(&params);

    KMP_close(&kmpClient);
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Completed receive thread");
    
    return NULL;
}


void start_receiver_server(struct ReceiverServer *server)
{
    server->totalOutputs=0;
    pthread_cond_init(&server->cond, NULL);
    pthread_mutex_init(&server->lock, NULL);
    pthread_create(&server->thread_id, NULL, listenerThread, server);
    pthread_cond_wait(&server->cond, &server->lock);
}


void stop_receiver_server(struct ReceiverServer *server) {
    
    KMP_close(&server->kmpServer);
    pthread_join(server->thread_id,NULL);
}

int get_receiver_stats(struct ReceiverServer *server,char* buf)
{
    char tmp[2048];
    JSON_SERIALIZE_INIT(buf)
    stats_to_json(&server->listnerStats, tmp);
    JSON_SERIALIZE_OBJECT("receiver", tmp)
    transcoding_context_to_json(server->transcodeContext,tmp);
    JSON_SERIALIZE_OBJECT("transcoder", tmp)
    JSON_SERIALIZE_END()
    return n;
}

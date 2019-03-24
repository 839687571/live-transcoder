//
//  listener.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "listener.h"
#include "utils.h"
#include "logger.h"
#include <pthread.h>
#include "config.h"
#include "KMP.h"


struct KalturaMediaProtocolContext kmpServer;
pthread_t thread_id;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct TranscodeOutput outputs[100];
int totalOutputs=0;

int init_outputs(struct TranscodeContext* pContext,json_value_t* json)
{
    const json_value_t* outputsJson;
    json_get(json,"outputs",&outputsJson);
    
    for (int i=0;i<json_get_array_count(outputsJson);i++)
    {
        const json_value_t outputJson;
        json_get_array_index(outputsJson,i,&outputJson);
        
        bool enabled=true;
        json_get_bool(&outputJson,"enabled",true,&enabled);
        if (!enabled) {
            char* name;
            json_get_string(&outputJson,"name","",&name);
            LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Skipping output %s since it's disabled",name);
            continue;
        }
        struct TranscodeOutput *pOutput=&outputs[totalOutputs];
        init_Transcode_output_from_json(pOutput,&outputJson);
        
        add_output(pContext,pOutput);
        totalOutputs++;
    }
    return 0;
}



void* listenerThread(void *vargp)
{
    
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"listenerThread");
    
    struct TranscodeContext *pContext = (struct TranscodeContext *)vargp;
    
    struct json_value_t* config=GetConfig();
    
    
    if (KMP_listen(&kmpServer,9999)<0) {
        exit (-1);
        return NULL;
    }
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Waiting for accept");
    pthread_cond_signal(&cond1);

    
    struct KalturaMediaProtocolContext kmpClient;

    if (KMP_accept(&kmpServer,&kmpClient)<0) {
        exit (-1);
        return NULL;
    }
    

    
    AVRational frameRate;
    
    AVCodecParameters* params=avcodec_parameters_alloc();
    if (KMP_read_mediaInfo(&kmpClient,params,&frameRate)<0) {
        LOGGER0(CATEGORY_RECEIVER,AV_LOG_FATAL,"Invalid mediainfo");
        exit (-1);
    }

    init_transcoding_context(pContext,params,frameRate);
    init_outputs(pContext,config);
    
    
    
    AVPacket packet;
    while (true) {
        
        if (KMP_readPacket(&kmpClient,&packet)<=0) {
            break;
        }

        LOGGER(CATEGORY_RECEIVER,AV_LOG_DEBUG,"[0] received packet %s",getPacketDesc(&packet));

        packet.pos=getClock64();
        convert_packet(pContext,&packet);
        
        av_packet_unref(&packet);
        
    }
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Destorying receive thread");

    close_transcoding_context(pContext);
    
    for (int i=0;i<totalOutputs;i++){
        LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Closing output %s",outputs[i].name);
        close_Transcode_output(&outputs[i]);
        
    }
    
    avcodec_parameters_free(&params);

    KMP_close(&kmpClient);
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Completed receive thread");
    
    return NULL;
}


void start_listener(struct TranscodeContext *pContext,int port)
{
    pthread_create(&thread_id, NULL, listenerThread, pContext);
    pthread_cond_wait(&cond1, &lock);
}


void stop_listener() {
    
    KMP_close(&kmpServer);
    pthread_join(thread_id,NULL);
}

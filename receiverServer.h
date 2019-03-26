//
//  listener.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef listener_h
#define listener_h

#include <stdio.h>
#include "TranscodePipeline.h"
#include "kalturaMediaProtocol.h"
#include "KMP.h"

struct ReceiverServer
{
    struct TranscodeContext *transcodeContext;
    struct KalturaMediaProtocolContext kmpServer;
    pthread_t thread_id;
    
    pthread_cond_t cond;
    pthread_mutex_t lock;
    
    struct TranscodeOutput outputs[100];
    int totalOutputs;
    struct FramesStats listnerStats;
};

void start_receiver_server(struct ReceiverServer *server);
void stop_receiver_server(struct ReceiverServer *server);
int get_receiver_stats(struct ReceiverServer *server,char* buf);

#endif /* listener_h */

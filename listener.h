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

typedef struct   {
    uint64_t dts;
    uint32_t pts_delay;
    uint32_t size;
    uint32_t flags;
} kaltura_network_frame_t;


void startService(struct TranscodeContext *pContext,int port);
void stopService();
#endif /* listener_h */

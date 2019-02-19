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

struct FrameHeader
{
    char header[4];
    size_t size;
    int64_t pts,dts,duration;
};

void startService(struct TranscodeContext *pContext,int port);
void stopService();
#endif /* listener_h */

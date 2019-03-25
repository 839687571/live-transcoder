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

void start_listener(struct TranscodeContext *pContext,int port);
void stop_listener();
int get_listener_stats(char* buf);

#endif /* listener_h */

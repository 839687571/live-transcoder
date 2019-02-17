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

void startService(struct TranscodeContext *pContext,int port);
#endif /* listener_h */

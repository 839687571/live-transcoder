//
//  sender.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 21/03/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef sender_h
#define sender_h

#include <stdio.h>
#include "kalturaMediaProtocol.h"

int init_sender_socket(int port);
int send_header(AVCodecParameters *codecpar,AVRational frame_rate);

int send_packet(AVPacket*);
int send_eof();

#endif /* sender_h */

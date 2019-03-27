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
#include <netinet/in.h>
//KMP
struct KalturaMediaProtocolContext
{
    int socket;
    struct sockaddr_in address;
};

int KMP_connect(struct KalturaMediaProtocolContext *context,char* url);
int KMP_send_header(struct KalturaMediaProtocolContext *context,AVCodecParameters *codecpar,AVRational frame_rate);

int KMP_send_packet(struct KalturaMediaProtocolContext *context,AVPacket*);
int KMP_send_eof(struct KalturaMediaProtocolContext *context);

int KMP_close(struct KalturaMediaProtocolContext *context);


int KMP_listen(struct KalturaMediaProtocolContext *context,int port);
int KMP_accept(struct KalturaMediaProtocolContext *context,struct KalturaMediaProtocolContext *client);
int KMP_read_header(struct KalturaMediaProtocolContext *context,packet_header_t *header);
int KMP_read_mediaInfo(struct KalturaMediaProtocolContext *context,packet_header_t *header,AVCodecParameters* params,AVRational *frameRate);
int KMP_readPacket(struct KalturaMediaProtocolContext *context,packet_header_t *header,AVPacket *packet);
#endif /* sender_h */

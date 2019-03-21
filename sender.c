//
//  sender.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 21/03/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "core.h"
#include "utils.h"
#include "logger.h"
#include "sender.h"

#include <netinet/in.h>
#include <arpa/inet.h>

int sock=0;

int init_sender_socket(int port)
{
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    return 0;
}



int send_header(AVCodecParameters *codecpar,AVRational frame_rate)
{
    if (sock==0)
    {
        return -1;
    }
    media_info_t mediaInfo;
    packet_header_t packetHeader;
    packetHeader.packet_type=PACKET_TYPE_HEADER;
    packetHeader.header_size=sizeof(mediaInfo);
    packetHeader.data_size=codecpar->extradata_size;
    mediaInfo.bitrate=(uint32_t)codecpar->bit_rate;
    mediaInfo.format=codecpar->codec_id;
    mediaInfo.timescale=90000;
    if (codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
    {
        mediaInfo.media_type=0;
        mediaInfo.u.video.width=codecpar->width;
        mediaInfo.u.video.height=codecpar->height;
        mediaInfo.u.video.sample_aspect_ratio.den=codecpar->sample_aspect_ratio.den;
        mediaInfo.u.video.sample_aspect_ratio.num=codecpar->sample_aspect_ratio.num;
        mediaInfo.u.video.frame_rate.den=frame_rate.den;
        mediaInfo.u.video.frame_rate.num=frame_rate.num;
    }
    if (codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
    {
        mediaInfo.media_type=1;
        mediaInfo.u.audio.bits_per_sample=codecpar->bits_per_raw_sample;
        mediaInfo.u.audio.sample_rate=codecpar->sample_rate;
        mediaInfo.u.audio.channels=codecpar->channels;
    }
    
    send(sock , &packetHeader , sizeof(packetHeader) , 0 );
    send(sock , &mediaInfo , sizeof(mediaInfo) , 0 );
    if (codecpar->extradata_size>0) {
        send(sock , codecpar->extradata , codecpar->extradata_size , 0 );
    }
    
    return 0;
}


int send_packet(AVPacket* packet)
{
    packet_header_t packetHeader;
    output_frame_t frame;
    
    packetHeader.packet_type=PACKET_TYPE_HEADER;
    packetHeader.header_size=sizeof(output_frame_t);
    packetHeader.data_size=packet->size;
    if (AV_NOPTS_VALUE!=packet->pts) {
        frame.pts_delay=(uint32_t)(packet->pts - packet->dts);
    } else {
        frame.pts_delay=-999999;
    }
    frame.dts=packet->dts;
    frame.flags=0;
    
    send(sock, &packetHeader, sizeof(packetHeader), 0);
    send(sock, &frame, sizeof(frame), 0);
    send(sock, packet->data, packet->size, 0);
    return 0;
}


int send_eof()
{
    packet_header_t packetHeader;
    packetHeader.packet_type=PACKET_TYPE_HEADER;
    packetHeader.header_size=0;
    packetHeader.data_size=0;
    send(sock, &packetHeader, sizeof(packetHeader), 0);
    
    close(sock);
    return 0;
}

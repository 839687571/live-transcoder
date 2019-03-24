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

#include "kalturaMediaProtocol.h"
#include "KMP.h"

#include <netinet/in.h>
#include <arpa/inet.h>


int KMP_connect(struct KalturaMediaProtocolContext *context,char* url,int port)
{
    
    context->socket=0;
    int ret=0;
    struct sockaddr_in serv_addr;
    if ((ret = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"Socket creation error %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    context->socket=ret;
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if((ret=inet_pton(AF_INET, url, &serv_addr.sin_addr))<=0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"Invalid address/ Address not supported (%s) %d (%s)",url,ret,av_err2str(ret));
        return ret;
    }
    
    if ( (ret=connect(context->socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"Connection Failed (%s) %d (%s)",url,ret,av_err2str(ret));
        return ret;
    }
    return 1;
}

int KMP_send_header(struct KalturaMediaProtocolContext *context,AVCodecParameters *codecpar,AVRational frame_rate)
{
    if (context->socket==0)
    {
        LOGGER0(CATEGORY_KMP,AV_LOG_FATAL,"Invalid socket");
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
    
    send(context->socket , &packetHeader , sizeof(packetHeader) , 0 );
    send(context->socket , &mediaInfo , sizeof(mediaInfo) , 0 );
    if (codecpar->extradata_size>0) {
        send(context->socket , codecpar->extradata , codecpar->extradata_size , 0 );
    }
    
    return 0;
}


int KMP_send_packet(struct KalturaMediaProtocolContext *context,AVPacket* packet)
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
    
    send(context->socket, &packetHeader, sizeof(packetHeader), 0);
    send(context->socket, &frame, sizeof(frame), 0);
    send(context->socket, packet->data, packet->size, 0);
    return 0;
}


int KMP_send_eof(struct KalturaMediaProtocolContext *context)
{
    packet_header_t packetHeader;
    packetHeader.packet_type=PACKET_TYPE_HEADER;
    packetHeader.header_size=0;
    packetHeader.data_size=0;
    send(context->socket, &packetHeader, sizeof(packetHeader), 0);
    
    return 0;
}

int KMP_close(struct KalturaMediaProtocolContext *context)
{
    close(context->socket);
    context->socket=0;
    return 0;
}

int KMP_listen(struct KalturaMediaProtocolContext *context,int port)
{
    int ret=0;
    // Creating socket file descriptor
    if ((ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"Socket creation error %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    
    context->socket =ret;
    
    /*
     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
     &opt, sizeof(opt)))
     {
     perror("setsockopt");
     exit(EXIT_FAILURE);
     }*/
    context->address.sin_family = AF_INET;
    context->address.sin_addr.s_addr = INADDR_ANY;
    context->address.sin_port = htons( port );
    
    // Forcefully attaching socket to the port
    if ( (ret=bind(context->socket, (struct sockaddr *)&context->address,sizeof(context->address)))<0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"bind error %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    if ( (ret=listen(context->socket, 10)) < 0)
    {
        LOGGER(CATEGORY_KMP,AV_LOG_FATAL,"listen failed %d (%s)",ret,av_err2str(ret));
        return ret;
    }
    return 0;
}

int KMP_accept(struct KalturaMediaProtocolContext *context,struct KalturaMediaProtocolContext *client)
{
    int addrlen = sizeof(context->address);
    int clientSocket=accept(context->socket, (struct sockaddr *)&context->address,
                            (socklen_t*)&addrlen);
    
    if (clientSocket<=0) {
        return clientSocket;
    }
    client->socket =clientSocket;
    return 1;
}


size_t recvEx(int socket,char* buffer,int bytesToRead) {
    
    size_t bytesRead=0;
    while (bytesToRead>0) {
        
        
        size_t valread = recv(socket,buffer+bytesRead, bytesToRead, 0);
        if (valread<0){
            return valread;
        }
        bytesRead+=valread;
        bytesToRead-=valread;
    }
    return bytesRead;
}

int KMP_read_mediaInfo(struct KalturaMediaProtocolContext *context,AVCodecParameters* params,AVRational *frameRate)
{
    packet_header_t header;
    media_info_t mediaInfo;

    size_t valread =recvEx(context->socket,(char*)&header,sizeof(header));

    if (header.packet_type!=PACKET_TYPE_HEADER) {
        
        exit(EXIT_FAILURE);
    }
    valread =recvEx(context->socket,(char*)&mediaInfo,sizeof(mediaInfo));

    if (mediaInfo.media_type==1) {
        params->codec_type=AVMEDIA_TYPE_AUDIO;
        params->sample_rate=mediaInfo.u.audio.sample_rate;
        params->bits_per_raw_sample=mediaInfo.u.audio.bits_per_sample;
        params->channels=mediaInfo.u.audio.channels;
        params->channel_layout=3;
    }
    if (mediaInfo.media_type==0) {
        params->codec_type=AVMEDIA_TYPE_VIDEO;
        params->format=AV_PIX_FMT_YUV420P;
        params->width=mediaInfo.u.video.width;
        params->height=mediaInfo.u.video.height;
        params->sample_aspect_ratio.den=mediaInfo.u.video.sample_aspect_ratio.den;
        params->sample_aspect_ratio.num=mediaInfo.u.video.sample_aspect_ratio.num;
        frameRate->den=mediaInfo.u.video.frame_rate.den;
        frameRate->num=mediaInfo.u.video.frame_rate.num;
        
    }
    params->bit_rate=mediaInfo.bitrate;
    params->codec_id=mediaInfo.format;
    params->extradata_size=header.data_size;
    params->extradata=NULL;
    if (params->extradata_size>0) {
        params->extradata=av_mallocz(params->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        valread =recvEx(context->socket,(char*)params->extradata,header.data_size);
    }
    
    return 0;
    
}

int KMP_readPacket(struct KalturaMediaProtocolContext *context,AVPacket *packet)
{
    packet_header_t header;
    output_frame_t networkFrame;
    
    size_t valread =recvEx(context->socket,(char*)&header,sizeof(header));
    
    if (header.data_size==0 && header.header_size==0) {
        return -1;
    }
    
    valread =recvEx(context->socket,(char*)&networkFrame,header.header_size);
    
    if (valread<0){
        return valread;
    }
    
    
    av_new_packet(packet,(int)header.data_size);
    packet->dts=networkFrame.dts;
    if (networkFrame.pts_delay!=-999999) {
        packet->pts=networkFrame.dts+networkFrame.pts_delay;
    } else {
        packet->pts=AV_NOPTS_VALUE;
    }
    packet->duration=0;
    
    valread =recvEx(context->socket,(char*)packet->data,(int)header.data_size);
    
    if (valread<0){
        return valread;
    }
    return 0;
}

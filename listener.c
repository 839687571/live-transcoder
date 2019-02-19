//
//  listener.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "listener.h"
#include <netinet/in.h>'
#include "logger.h"

pthread_t thread_id;


int recvEx(int socket,char* buffer,int bytesToRead) {
    
    int bytesRead=0;
    while (bytesToRead>0) {
        
        
        int valread = recv( socket , buffer+bytesRead, bytesToRead, 0);
        if (valread<0){
            return valread;
        }
        bytesRead+=valread;
        bytesToRead-=valread;
    }
    return bytesRead;
}


void* listenerThread(void *vargp)
{
    struct TranscodeContext *pContext = (struct TranscodeContext *)vargp;
    int port=9999;
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    /*
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }*/
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );
    
    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    struct FrameHeader frameHeader;
    AVPacket packet;
    
    while (true) {
        
        int valread =recvEx(new_socket,&frameHeader,sizeof(frameHeader));
        
        if (valread<0){
            break;
        }
        
        av_new_packet(&packet,frameHeader.size);
        packet.pts=frameHeader.pts;
        packet.dts=frameHeader.dts;
        packet.duration=frameHeader.duration;
        
        valread =recvEx(new_socket,packet.data,frameHeader.size);
        
        if (valread<0){
            break;
        }
        LOGGER("RECEIVER",AV_LOG_DEBUG,"received packet pts=%s dts=%s size=%d",
               ts2str(packet.pts,true),
               ts2str(packet.dts,true),
               packet.size);

        convert_packet(pContext,&packet);
        
    }
    return NULL;
}


void startService(struct TranscodeContext *pContext,int port)
{
    pthread_create(&thread_id, NULL, listenerThread, pContext);
}


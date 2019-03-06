//
//  listener.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 17/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "listener.h"
#include <netinet/in.h>
#include "utils.h"
#include "logger.h"
#include <pthread.h>
#include "config.h"

pthread_t thread_id;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


struct TranscodeOutput outputs[100];
int totalOutputs=0;

int init_outputs(struct TranscodeContext* pContext,json_value_t* json)
{
    const json_value_t* outputsJson;
    json_get(json,"outputs",&outputsJson);
    
    for (int i=0;i<json_get_array_count(outputsJson);i++)
    {
        const json_value_t outputJson;
        json_get_array_index(outputsJson,i,&outputJson);
        
        struct TranscodeOutput *pOutput=&outputs[totalOutputs];
        init_Transcode_output_from_json(pOutput,&outputJson);
        
        add_output(pContext,pOutput);
        totalOutputs++;
    }
    return 0;
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


void* listenerThread(void *vargp)
{
    
    struct TranscodeContext *pContext = (struct TranscodeContext *)vargp;
    
    struct json_value_t* config=GetConfig();
    
    int port=9999;
    
    int server_fd, new_socket;
    struct sockaddr_in address;
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
    pthread_cond_signal(&cond1);

    
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    packet_header_t header;
    media_info_t mediaInfo;

    size_t valread =recvEx(new_socket,(char*)&header,sizeof(header));
    
    if (header.packet_type!=PACKET_TYPE_HEADER) {
        
        exit(EXIT_FAILURE);
    }
    valread =recvEx(new_socket,(char*)&mediaInfo,sizeof(mediaInfo));
    
    AVCodecParameters params;
    params.bit_rate=mediaInfo.bitrate;
    params.width=mediaInfo.u.video.width;
    params.height=mediaInfo.u.video.height;
    params.codec_id=mediaInfo.format;
    params.extradata_size=header.data_size;
    params.extradata=NULL;
    if (params.extradata_size>0) {
         params.extradata=av_mallocz(params.extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        valread =recvEx(new_socket,(char*)params.extradata,header.data_size);
    }
    init_transcoding_context(pContext,&params);
    init_outputs(pContext,config);
    
    
    output_frame_t networkFrame;
    AVPacket packet;
    
    while (true) {
        
        packet_header_t header;
        valread =recvEx(new_socket,(char*)&header,sizeof(header));
        valread =recvEx(new_socket,(char*)&networkFrame,header.header_size);
        
        if (valread<0){
            break;
        }
        
        if (networkFrame.flags==99) {
            close_transcoding_context(pContext);
            break;
        }
        
        av_new_packet(&packet,(int)header.data_size);
        packet.dts=networkFrame.dts;
        if (networkFrame.pts_delay!=-999999) {
            packet.pts=networkFrame.dts+networkFrame.pts_delay;
        } else {
            packet.pts=AV_NOPTS_VALUE;
        }
        packet.duration=0;
        
        valread =recvEx(new_socket,(char*)packet.data,(int)header.data_size);
        
        if (valread<0){
            break;
        }
        LOGGER("RECEIVER",AV_LOG_DEBUG,"received packet pts=%s dts=%s size=%d",
               ts2str(packet.pts,true),
               ts2str(packet.dts,true),
               packet.size);

        packet.pos=getClock64();
        convert_packet(pContext,&packet);
        
    }
    
    
    for (int i=0;i<totalOutputs;i++){
        close_Transcode_output(&outputs[i]);
        
    }

    
    return NULL;
}


void startService(struct TranscodeContext *pContext,int port)
{
    pthread_create(&thread_id, NULL, listenerThread, pContext);
    pthread_cond_wait(&cond1, &lock);

}


void stopService() {
    pthread_join(&thread_id,NULL);
}

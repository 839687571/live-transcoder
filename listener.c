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
        
        bool enabled=true;
        json_get_bool(&outputJson,"enabled",true,&enabled);
        if (!enabled) {
            char* name;
            json_get_string(&outputJson,"name","",&name);
            LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Skipping output %s since it's disabled",name);
            continue;
        }
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
    
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"listenerThread");
    
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
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Waiting for accept");
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
    
    AVRational frameRate;
    AVCodecParameters* params=avcodec_parameters_alloc();
    if (mediaInfo.media_type==1) {
        params->codec_type=AVMEDIA_TYPE_AUDIO;
        params->sample_rate=mediaInfo.u.audio.sample_rate;
        params->bits_per_raw_sample=mediaInfo.u.audio.bits_per_sample;
        params->channels=mediaInfo.u.audio.channels;
    }
    if (mediaInfo.media_type==0) {
        params->codec_type=AVMEDIA_TYPE_VIDEO;
        params->format=AV_PIX_FMT_YUV420P;
        params->width=mediaInfo.u.video.width;
        params->height=mediaInfo.u.video.height;
        AVRational sampleRatio={1,1};
        params->sample_aspect_ratio.den=mediaInfo.u.video.sample_aspect_ratio.den;
        params->sample_aspect_ratio.num=mediaInfo.u.video.sample_aspect_ratio.num;
        frameRate.den=mediaInfo.u.video.frame_rate.den;
        frameRate.num=mediaInfo.u.video.frame_rate.num;
        
    }
    params->bit_rate=mediaInfo.bitrate;
    params->codec_id=mediaInfo.format;
    params->extradata_size=header.data_size;
    params->extradata=NULL;
    if (params->extradata_size>0) {
         params->extradata=av_mallocz(params->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        valread =recvEx(new_socket,(char*)params->extradata,header.data_size);
    }
    init_transcoding_context(pContext,params,frameRate);
    init_outputs(pContext,config);
    
    
    output_frame_t networkFrame;
    
    AVPacket packet;
    while (true) {
        
        packet_header_t header;
        valread =recvEx(new_socket,(char*)&header,sizeof(header));
        
        if (header.data_size==0 && header.header_size==0) {
            close_transcoding_context(pContext);
            break;
        }
        
        valread =recvEx(new_socket,(char*)&networkFrame,header.header_size);
        
        if (valread<0){
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
        LOGGER(CATEGORY_RECEIVER,AV_LOG_DEBUG,"[0] received packet %s",
               getPacketDesc(&packet));

        packet.pos=getClock64();
        convert_packet(pContext,&packet);
        
        av_packet_unref(&packet);
        
    }
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Destorying receive thread");

    
    for (int i=0;i<totalOutputs;i++){
        LOGGER(CATEGORY_RECEIVER,AV_LOG_INFO,"Closing output %s",outputs[i].name);
        close_Transcode_output(&outputs[i]);
        
    }
    
    avcodec_parameters_free(&params);

    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Completed receive thread");
    
    return NULL;
}


void start_listener(struct TranscodeContext *pContext,int port)
{
    pthread_create(&thread_id, NULL, listenerThread, pContext);
    pthread_cond_wait(&cond1, &lock);

}


void stop_listener() {
    pthread_join(thread_id,NULL);
}

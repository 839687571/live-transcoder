#define __STDC_CONSTANT_MACROS

#include "core.h"
#include <libavutil/timestamp.h>
        
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include "logger.h"
#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif

#include "TranscodePipeline.h"
#include "listener.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "output.h"
#include "json_parser.h"
#include "utils.h"
#include "config.h"
#include <unistd.h>
#include <signal.h>

int sock=0;



int init_socket(int port)
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


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_WARNING,"SIGINT detected!");
    keepRunning = 0;
}

int main(int argc, char **argv)
{
    log_init(AV_LOG_DEBUG);
    signal(SIGINT, intHandler);


    int ret=LoadConfig(argc,argv);
    if (ret < 0) {
        return ret;
    }

    char* pSourceFileName;
    json_get_string(GetConfig(),"input.file","",&pSourceFileName);

    int numberOfFrames=0;
    json_get_int(GetConfig(),"input.numberOfFrames",-1,&numberOfFrames);
    
    bool realTime;
    json_get_bool(GetConfig(),"input.realTime",false,&realTime);
    
    int activeStream=0;
    json_get_int(GetConfig(),"activeStream",0,&activeStream);
    int randomDataPercentage;
    json_get_int(GetConfig(),"input.randomDataPercentage",0,&randomDataPercentage);


    AVFormatContext *ifmt_ctx=NULL;
    ret = avformat_open_input(&ifmt_ctx, pSourceFileName, NULL, NULL);
    if (ret < 0) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_FATAL,"Unable to open input %s %d (%s)",pSourceFileName,ret,av_err2str(ret));
        return ret;
        
    }
    ret = avformat_find_stream_info(ifmt_ctx, NULL);
    if (ret < 0) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_FATAL,"segmenter: Unable to find any input streams  %d (%s)",ret,av_err2str(ret));
        return ret;
    }

    //LOGGER("Version: %s\n", VERSION);

    avformat_network_init();
    
    init_ffmpeg_log_level(AV_LOG_DEBUG);
    
    struct TranscodeContext ctx;
    

    
    startService(&ctx,9999);
    AVPacket packet;
    av_init_packet(&packet);
    
    init_socket(9999);
    uint64_t  basePts=0;//av_rescale_q( getClock64(), clockScale, standard_timebase);
    
    AVStream *in_stream=ifmt_ctx->streams[activeStream];
    media_info_t mediaInfo;
    packet_header_t packetHeader;
    
    if (sock!=0)
    {
        packetHeader.packet_type=PACKET_TYPE_HEADER;
        packetHeader.header_size=sizeof(mediaInfo);
        packetHeader.data_size=in_stream->codecpar->extradata_size;
        mediaInfo.bitrate=in_stream->codecpar->bit_rate;
        mediaInfo.format=in_stream->codecpar->codec_id;
        mediaInfo.timescale=90000;
        if (in_stream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            mediaInfo.media_type=0;
            mediaInfo.u.video.width=in_stream->codecpar->width;
            mediaInfo.u.video.height=in_stream->codecpar->height;
            mediaInfo.u.video.sample_aspect_ratio.den=in_stream->codecpar->sample_aspect_ratio.den;
            mediaInfo.u.video.sample_aspect_ratio.num=in_stream->codecpar->sample_aspect_ratio.num;
            mediaInfo.u.video.frame_rate.den=1;
            mediaInfo.u.video.frame_rate.num=15;
        }
        if (in_stream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO)
        {
            mediaInfo.media_type=1;
            mediaInfo.u.audio.bits_per_sample=in_stream->codecpar->bits_per_raw_sample;
            mediaInfo.u.audio.sample_rate=in_stream->codecpar->sample_rate;
            mediaInfo.u.audio.channels=in_stream->codecpar->channels;
        }
        
        send(sock , &packetHeader , sizeof(packetHeader) , 0 );
        send(sock , &mediaInfo , sizeof(mediaInfo) , 0 );
        if (in_stream->codecpar->extradata_size>0) {
            send(sock , in_stream->codecpar->extradata , in_stream->codecpar->extradata_size , 0 );
        }
    }
    LOGGER("SENDER",AV_LOG_INFO,"Realtime = %s",realTime ? "true" : "false");
    srand(time(NULL));
    uint64_t lastDts=0;
    int64_t start_time=av_gettime_relative();
    while (keepRunning && !kbhit()) {
        
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
        {
            av_seek_frame(ifmt_ctx,activeStream,0,AVSEEK_FLAG_FRAME);
            basePts+=lastDts;
            continue;
        }
        
        if (activeStream!=packet.stream_index) {
            av_packet_unref(&packet);
            continue;
        }
        
        if (numberOfFrames!=-1) {
            numberOfFrames--;
            if (numberOfFrames<0) {
                break;
            }
        }
        
        AVStream *in_stream=ifmt_ctx->streams[packet.stream_index];
        
        av_packet_rescale_ts(&packet,in_stream->time_base, standard_timebase);
        
        if (sock!=0)
        {
            packetHeader.packet_type=PACKET_TYPE_HEADER;
            packetHeader.header_size=sizeof(output_frame_t);
            packetHeader.data_size=packet.size;
            output_frame_t frame;
            if (AV_NOPTS_VALUE!=packet.pts) {
                frame.pts_delay=packet.pts-packet.dts;
            } else {
                frame.pts_delay=-999999;
            }
            frame.dts=packet.dts+basePts;
            frame.flags=0;
            lastDts=packet.dts;
            
            
            
            if (realTime==true) {
                
                int64_t timePassed=av_rescale_q(frame.dts-basePts,standard_timebase,AV_TIME_BASE_Q);
                //LOGGER("SENDER",AV_LOG_DEBUG,"XXXX dt=%ld dd=%ld", (av_gettime_relative() - start_time),timePassed);
                while ((av_gettime_relative() - start_time) < timePassed) {
                    
                    // LOGGER0("SENDER",AV_LOG_DEBUG,"XXXX Sleep 10ms");
                    av_usleep(10*1000);//10ms
                }
            } 
            
            send(sock, &packetHeader, sizeof(packetHeader), 0);
            send(sock, &frame, sizeof(frame), 0);
            if (randomDataPercentage>0 && ((rand() % 100) < randomDataPercentage)) {
                LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"random!");
                for (int i=0;i<packet.size;i++) {
                    packet.data[i]=rand();
                }
            }
            send(sock, packet.data, packet.size, 0);
        }
        /*
        LOGGER("SENDER",AV_LOG_DEBUG,"sent packet pts=%s dts=%s  size=%d",
               ts2str(packet.pts,true),
               ts2str(packet.dts,true),
               packet.dts,packet.size);*/

        
        av_packet_unref(&packet);

    }
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"stopping!");
    
    packetHeader.packet_type=PACKET_TYPE_HEADER;
    packetHeader.header_size=0;
    packetHeader.data_size=0;
    send(sock, &packetHeader, sizeof(packetHeader), 0);


    close(sock);
    stopService();
    
    avformat_close_input(&ifmt_ctx);
    
    loggerFlush();
    return 0;
}


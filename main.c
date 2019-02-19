#define __STDC_CONSTANT_MACROS


#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
        
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

int sock=0;

void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if (level<AV_LOG_DEBUG)
        return;
    logger2("FFMPEG",level,fmt,vargs);
}


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

int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);

  //  av_log_set_callback(ffmpeg_log_callback);
    
    //char* pSourceFileName="/Users/guyjacubovski/Sample_video/קישון - תעלת בלאומילך.avi";
    char* pSourceFileName="/Users/guyjacubovski/Sample_video/900.mp4";

    AVFormatContext *ifmt_ctx;
    int ret = avformat_open_input(&ifmt_ctx, pSourceFileName, NULL, NULL);
    
    if (ret < 0) {
        char buff[256];
        av_strerror(ret, buff, 256);
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
    
    struct TranscodeContext ctx;
    
    int activeStream=0;
    

    init_transcoding_context(&ctx,ifmt_ctx->streams[activeStream]->codecpar);

    
    struct TranscodeOutput output32;
    init_Transcode_output(&output32);
    struct TranscodeOutput output33;
    init_Transcode_output(&output33);
    struct TranscodeOutput output34;
    init_Transcode_output(&output34);
    if (activeStream==0)
    {

        output32.name="32";
        output32.codec_type=AVMEDIA_TYPE_VIDEO;
        output32.passthrough=true;
        add_output(&ctx,&output32);

        output33.name="33";
        output33.codec_type=AVMEDIA_TYPE_VIDEO;
        output33.passthrough=false;
        output33.videoParams.width=352;
        output33.videoParams.height=240;
        output33.videoParams.fps=30;
        output33.bitrate=500;
        add_output(&ctx,&output33);

        output34.name="34";
        output34.codec_type=AVMEDIA_TYPE_VIDEO;
        output34.passthrough=false;
        output34.videoParams.width=352;
        output34.videoParams.height=240;
        output34.videoParams.fps=30;
        output34.bitrate=200;

        //add_output(&ctx,&output34);
    }
    if (activeStream==1)
    {
        
        output32.name="32";
        output32.codec_type=AVMEDIA_TYPE_AUDIO;
        output32.passthrough=true;
        
        //add_output(&ctx,&output32);
        
        output33.name="33";
        output33.codec_type=AVMEDIA_TYPE_AUDIO;
        output33.passthrough=false;
        output33.audioParams.samplingRate=44100;
        output33.bitrate=128;
        
        add_output(&ctx,&output33);
        
        output34.name="34";
        output33.codec_type=AVMEDIA_TYPE_AUDIO;
        output33.passthrough=false;
        output33.audioParams.samplingRate=44100;
        output33.bitrate=64;
        
        //add_output(&ctx,&output34);
    }


    startService(&ctx,9999);
    AVPacket packet;
    av_init_packet(&packet);
    
    init_socket(9999);
    
    uint64_t  basePts=0;//(60LL*60LL)*(100LL*24LL-2LL)*1000LL;//getTime64();
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        
        if (activeStream!=packet.stream_index) {
            continue;
        }
        
        AVStream *in_stream=ifmt_ctx->streams[packet.stream_index];
        
        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, standard_timebase, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, standard_timebase, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, standard_timebase);
        
       // convert_packet(&ctx,&packet);
        
        struct FrameHeader header;
        header.size=packet.size;
        header.pts=packet.pts+basePts;
        header.dts=packet.dts+basePts;
        header.duration=packet.duration;
        header.header[0]=1;
        header.header[1]=2;
        header.header[2]=3;
        header.header[3]=4;
        send(sock , &header , sizeof(header) , 0 );
        send(sock, packet.data,packet.size,0);
        /*
        LOGGER("SENDER",AV_LOG_DEBUG,"sent packet pts=%s dts=%s  size=%d",
               ts2str(header.pts,true),
               ts2str(header.dts,true),
               packet.dts,packet.size);*/


    }
    return 0;
}


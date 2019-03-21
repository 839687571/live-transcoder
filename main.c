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
#include "output.h"
#include "json_parser.h"
#include "utils.h"
#include "config.h"
#include <unistd.h>
#include <signal.h>
#include "sender.h"



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
    
    init_sender_socket(9999);
    uint64_t  basePts=0;//av_rescale_q( getClock64(), clockScale, standard_timebase);
    
    AVStream *in_stream=ifmt_ctx->streams[activeStream];
    
    AVRational frame_rate={1,15};
    send_header(in_stream->codecpar,frame_rate);
    
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
        packet.pts+=basePts;
        packet.dts+=basePts;
        
        if (randomDataPercentage>0 && ((rand() % 100) < randomDataPercentage)) {
            LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"random!");
            for (int i=0;i<packet.size;i++) {
                packet.data[i]=rand();
            }
        }
        
        if (realTime) {
                
            int64_t timePassed=av_rescale_q(packet.dts-basePts,standard_timebase,AV_TIME_BASE_Q);
            //LOGGER("SENDER",AV_LOG_DEBUG,"XXXX dt=%ld dd=%ld", (av_gettime_relative() - start_time),timePassed);
            while ((av_gettime_relative() - start_time) < timePassed) {
                
                // LOGGER0("SENDER",AV_LOG_DEBUG,"XXXX Sleep 10ms");
                av_usleep(10*1000);//10ms
            }
        }
            
        lastDts=packet.dts;
        send_packet(&packet);
        
        /*
        LOGGER("SENDER",AV_LOG_DEBUG,"sent packet pts=%s dts=%s  size=%d",
               ts2str(packet.pts,true),
               ts2str(packet.dts,true),
               packet.dts,packet.size);*/

        
        av_packet_unref(&packet);

    }
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"stopping!");
    
    send_eof();


    stopService();
    
    avformat_close_input(&ifmt_ctx);
    
    loggerFlush();
    return 0;
}


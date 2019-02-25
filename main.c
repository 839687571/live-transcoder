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

int sock=0;

struct TranscodeOutput outputs[100];
int totalOutputs=0;

void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    return;
    if (level<AV_LOG_INFO)
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


int init_outputs(struct TranscodeContext* pContext,json_value_t* json)
{
    json_value_t* outputsJson;
    json_get(json,"outputs",&outputsJson);
    
    for (int i=0;i<json_get_array_count(outputsJson);i++)
    {
        json_value_t outputJson;
        json_get_array_index(outputsJson,i,&outputJson);
        
        struct TranscodeOutput *pOutput=&outputs[totalOutputs];
        init_Transcode_output_from_json(pOutput,&outputJson);
        
        add_output(pContext,pOutput);
        totalOutputs++;
    }
    return 0;
}
int main(int argc, char **argv)
{

    int ret=LoadConfig();
    if (ret < 0) {
        return ret;
    }

    char* pSourceFileName;
    json_get_string(GetConfig(),"input","",&pSourceFileName);

    av_log_set_level(AV_LOG_DEBUG);
    av_log_set_callback(ffmpeg_log_callback);
    


    AVFormatContext *ifmt_ctx;
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
    
    struct TranscodeContext ctx;
    
    int activeStream=0;
    

    init_transcoding_context(&ctx,ifmt_ctx->streams[activeStream]->codecpar);

    init_outputs(&ctx,GetConfig());

    startService(&ctx,9999);
    AVPacket packet;
    av_init_packet(&packet);
    
    init_socket(9999);
    uint64_t  basePts=av_rescale_q( getClock64(), clockScale, standard_timebase);
    while (!kbhit()) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0)
            break;
        
        if (activeStream!=packet.stream_index) {
            continue;
        }
        
        AVStream *in_stream=ifmt_ctx->streams[packet.stream_index];
        
        av_packet_rescale_ts(&packet,in_stream->time_base, standard_timebase);
        
       // convert_packet(&ctx,&packet);
        
        kaltura_network_frame_t frame;
        frame.size=packet.size;
        if (AV_NOPTS_VALUE!=packet.pts) {
            frame.pts_delay=packet.pts-packet.dts;
        } else {
            frame.pts_delay=-999999;
        }
        frame.dts=packet.dts+basePts;
        frame.flags=0;
        send(sock , &frame , sizeof(frame) , 0 );
        send(sock, packet.data,packet.size,0);
        /*
        LOGGER("SENDER",AV_LOG_DEBUG,"sent packet pts=%s dts=%s  size=%d",
               ts2str(header.pts,true),
               ts2str(header.dts,true),
               packet.dts,packet.size);*/

        av_packet_unref(&packet);

    }
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"stopping!");

    kaltura_network_frame_t frame;
    frame.flags=999;
    send(sock , &frame , sizeof(frame) , 0 );
    
    stopService();
    
    for (int i=0;i<totalOutputs;i++){
        close_Transcode_output(&outputs[i]);

    }

    avformat_close_input(&ifmt_ctx);
    
    return 0;
}


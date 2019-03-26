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
#include "receiverServer.h"
#include "output.h"
#include "json_parser.h"
#include "utils.h"
#include "config.h"
#include <unistd.h>
#include <signal.h>
#include "fileReader.h"
#include "httpServer.h"

#ifndef APPLICATION_VERSION
#define APPLICATION_VERSION __TIMESTAMP__
#endif

static volatile bool keepRunning = true;

struct TranscodeContext ctx;

void intHandler(int dummy) {
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_WARNING,"SIGINT detected!");
    keepRunning = false;
}


int on_http_request(const char* uri, char* buf,int bufSize,int* bytesWritten)
{
    int retVal=404;
    JSON_SERIALIZE_INIT(buf)
    JSON_SERIALIZE_STRING("uri", uri)
    
    char tmp[2048];
    strcpy(tmp,"{}");
    if (strcmp(uri,"/stats")==0) {
        transcoding_context_to_json(&ctx,tmp);
        retVal=200;
    }
    
    JSON_SERIALIZE_OBJECT("result", tmp);
    
    JSON_SERIALIZE_END()
    *bytesWritten=n;

    return retVal;
}


int main(int argc, char **argv)
{
    log_init(AV_LOG_DEBUG);
    
    LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Version: %s", APPLICATION_VERSION)

    signal(SIGINT, intHandler);

    int ret=LoadConfig(argc,argv);
    if (ret < 0) {
        return ret;
    }

    char* pSourceFileName;
    json_get_string(GetConfig(),"input.file","",&pSourceFileName);
    
    init_ffmpeg_log_level(AV_LOG_DEBUG);
    avformat_network_init();
    
    
    int listenPort;
    json_get_int(GetConfig(),"listener.port",9999,&listenPort);

    start_http_server(12345, on_http_request);
    
    struct ReceiverServer receiver;
    receiver.transcodeContext=&ctx;
    start_receiver_server(&receiver);
    
    if (strlen(pSourceFileName)>0)
    {
        if ( (ret=stream_from_file(pSourceFileName,&keepRunning))<0) {
            return ret;
        }
    }
    else
    {
        while (keepRunning && !kbhit()) {
            av_usleep(10000);
        }
    }
    
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_INFO,"stopping!");
    
    stop_receiver_server(&receiver);
    
    stop_http_server();
    
    
    loggerFlush();
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_INFO,"exiting");

    return 0;
}


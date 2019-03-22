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
#include "fileReader.h"

#ifndef APPLICATION_VERSION
#define APPLICATION_VERSION __TIMESTAMP__
#endif

static volatile bool keepRunning = true;

void intHandler(int dummy) {
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_WARNING,"SIGINT detected!");
    keepRunning = false;
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
    
    struct TranscodeContext ctx;
    
    int listenPort;
    json_get_int(GetConfig(),"listener.port",9999,&listenPort);

    
    start_listener(&ctx,listenPort);
    
    if (strlen(pSourceFileName)>0)
    {
        if ( (ret=stream_from_file(pSourceFileName,&keepRunning))<0) {
            return ret;
        }
    }
    else
    {
    }
    
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_INFO,"stopping!");
    
    stop_listener();
    
    
    loggerFlush();
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_INFO,"exiting");

    return 0;
}


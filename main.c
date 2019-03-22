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

static volatile bool keepRunning = true;

void intHandler(int dummy) {
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_WARNING,"SIGINT detected!");
    keepRunning = false;
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




    //LOGGER("Version: %s\n", VERSION);

    avformat_network_init();
    
    init_ffmpeg_log_level(AV_LOG_DEBUG);
    
    struct TranscodeContext ctx;
    

    
    startService(&ctx,9999);
    
    if (strlen(pSourceFileName)>0)
    {
        if ( (ret=stream_from_file(pSourceFileName,&keepRunning))<0) {
            return ret;
        }
    }
    else
    {
    }
    LOGGER0(CATEGORY_DEFAULT,AV_LOG_FATAL,"stopping!");
    
    send_eof();


    stopService();
    
    
    loggerFlush();
    return 0;
}


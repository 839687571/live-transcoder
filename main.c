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
#include "input.h"

#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};

#include "TranscodePipeline.h"





int main(int argc, char **argv)
{
    av_log_set_level(AV_LOG_DEBUG);


    //logger("Version: %s\n", VERSION);

    avformat_network_init();
    
    struct InputContext inputCtx;
    init_input(&inputCtx,"/Users/guyjacubovski/Sample_video/קישון - תעלת בלאומילך.avi");

    
    struct TranscodeContext ctx;
    init_transcoding_context(&ctx,&inputCtx);


    struct TranscodeOutput output1;
    init_Transcode_output(&output1);
    add_output(&ctx,&output1);
 
    
    process(&inputCtx);
    return 0;
}


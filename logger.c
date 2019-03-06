//
//  LOGGER.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include <stdio.h>

#include <stdbool.h>
#include <sys/time.h>
#include "logger.h"
#include "utils.h"


static int logLevel =AV_LOG_VERBOSE;

const   char* getLevel(int level) {
    switch(level){
        case AV_LOG_PANIC: return "PANIC";
        case AV_LOG_FATAL: return "FATAL";
        case AV_LOG_ERROR: return "ERROR";
        case AV_LOG_WARNING: return "WARN";
        case AV_LOG_INFO: return "INFO";
        case AV_LOG_VERBOSE: return "VERBOSE";
        case AV_LOG_DEBUG: return "DEBUG";
    }
    return "";
}
void logger2(char* category,int level,const char *fmt, bool newLine, va_list args)
{    
    const char* levelStr=getLevel(level);
    
    int64_t now=getClock64();
    time_t epoch=now/1000000;
    struct tm *gm = localtime(&epoch);
    
    
    char buf[25];
    strftime(buf, 25, "%Y-%m-%dT%H:%M:%S",gm);
    
    
    fprintf( stderr, "%s.%03d %s %s ",buf,(int)( (now % 1000000)/1000 ),category, levelStr);
    vfprintf( stderr, fmt, args );
    if (newLine) {
        fprintf( stderr, "\n" );
    }
}



void logger1(char* category,int level,const char *fmt, ...)
{
    va_list args;
    va_start( args, fmt );
    logger2(category,level,fmt,true,args);
    va_end( args );
}


const char* pict_type_to_string(int pt) {
    
    const char *pict_type;
    switch (pt)
    {
        case AV_PICTURE_TYPE_I: pict_type="I"; break;     ///< Intra
        case AV_PICTURE_TYPE_P: pict_type="P"; break;      ///< Predicted
        case AV_PICTURE_TYPE_B: pict_type="B"; break;      ///< Bi-dir predicted
        case AV_PICTURE_TYPE_S: pict_type="S"; break;      ///< S(GMC)-VOP MPEG-4
        case AV_PICTURE_TYPE_SI: pict_type="SI"; break;     ///< Switching Intra
        case AV_PICTURE_TYPE_SP: pict_type="SP"; break;     ///< Switching Predicted
        case AV_PICTURE_TYPE_BI: pict_type="BI"; break;     ///< BI type
        default: pict_type="";
    }
    return pict_type;
}
/*
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    
    LOGGER(AV_LOG_DEBUG,"%s:  stream_index:%d  pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s flags:%d\n",
           tag,
           pkt->stream_index,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->flags);
}*/


void ffmpeg_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
{
    if (level>logLevel)
        return;
    logger2(CATEGORY_FFMPEG,level,fmt,false,vargs);
}


void log_init(int level)
{
    logLevel=level;
    av_log_set_level(AV_LOG_INFO);
    av_log_set_callback(ffmpeg_log_callback);
}
int get_log_level(char* category,int level)
{
    return logLevel;
}
void loggerFlush() 
{
    fflush(stderr);
}

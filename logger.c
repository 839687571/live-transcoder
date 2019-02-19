//
//  LOGGER.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#include <stdio.h>
#include <libavformat/avformat.h>

#include <sys/ioctl.h> // For FIONREAD
#include <termios.h>
#include <stdbool.h>
#include <sys/time.h>
#include "logger.h"




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
void logger2(char* category,int level,const char *fmt, va_list args)
{    
    const char* levelStr=getLevel(level);
    fprintf( stderr, "%s %s %s ",ts2str(getTime64(),false),category, levelStr);

    vfprintf( stderr, fmt, args );
    fprintf( stderr, "\n" );
}



void logger1(char* category,int level,const char *fmt, ...)
{
    va_list args;
    va_start( args, fmt );
    logger2(category,level,fmt,args);
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

int kbhit(void)
{
    static bool initflag = false;
    static const int STDIN = 0;
    
    if (!initflag) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initflag = true;
    }
    
    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
    return nbbytes;
}


uint64_t getTime64()
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    unsigned long long millisecondsSinceEpoch =
    (unsigned long long)(tv.tv_sec) * 1000 +
    (unsigned long long)(tv.tv_usec) / 1000;

    return millisecondsSinceEpoch;
}

char *av_ts_make_time_stringEx(char *buf, int64_t ts,bool shortFormat)
{
    
    if (ts == AV_NOPTS_VALUE) {
        snprintf(buf, K_TS_MAX_STRING_SIZE, "NOPTS");
        return buf;
    }

    time_t epoch=ts/1000;
    struct tm *gm = localtime(&epoch);

    
    ssize_t written = (ssize_t)strftime(buf, K_TS_MAX_STRING_SIZE, shortFormat ? "%H:%M:%S" : "%Y-%m-%dT%H:%M:%S", gm);
    if ((written > 0) && ((size_t)written < K_TS_MAX_STRING_SIZE))
    {
        int w = snprintf(buf+written, K_TS_MAX_STRING_SIZE-(size_t)written, ".%03d", ts % 1000);

    }
    return buf;
}

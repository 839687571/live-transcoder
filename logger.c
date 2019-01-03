//
//  logger.c
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

void logger(int level,const char *fmt, ...)
{
    va_list args;
    fprintf( stderr, "LOG: " );
    
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
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
    
    logger(AV_LOG_DEBUG,"%s:  stream_index:%d  pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s flags:%d\n",
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

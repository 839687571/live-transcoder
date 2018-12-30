#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <sys/ioctl.h> // For FIONREAD
#include <termios.h>
#include <stdbool.h>


#ifndef VERSION
#define VERSION __TIMESTAMP__
#endif
static  AVRational standard_timebase = {1,1000};




static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    
    printf("%s:  stream_index:%d  pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s flags:%d\n",
           tag,
           pkt->stream_index,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),           pkt->flags);
}

int kbhit(void) {
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



int main(int argc, char **argv)
{
    
    
    printf("Version: %s\n", VERSION);

    avformat_network_init();
    
    return 0;
}


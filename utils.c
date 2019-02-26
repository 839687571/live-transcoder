//
//  utils.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 22/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "utils.h"
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h> // For FIONREAD

int load_file_to_memory(const char *filename, char **result)
{
    int size = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        *result = NULL;
        return -1; // -1 means file opening fail
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *result = (char *)malloc(size+1);
    if (size != fread(*result, sizeof(char), size, f))
    {
        //free(*result);
        return -2; // -2 means file reading fail
    }
    fclose(f);
    (*result)[size] = 0;
    return size;
}


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
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    
    
    uint64_t usecondsSinceEpoch =
    (uint64_t)(ts.tv_sec) * 1000000 +
    (uint64_t)(ts.tv_nsec) / 1000;
    
    return usecondsSinceEpoch;
}

uint64_t getClock64()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    
    uint64_t usecondsSinceEpoch =
    (uint64_t)(ts.tv_sec) * 1000000 +
    (uint64_t)(ts.tv_nsec) / 1000;
    
    return usecondsSinceEpoch;
}

char *av_ts_make_time_stringEx(char *buf, int64_t ts,bool shortFormat)
{
    
    if (ts == AV_NOPTS_VALUE) {
        snprintf(buf, K_TS_MAX_STRING_SIZE, "NOPTS");
        return buf;
    }
    
    time_t epoch=ts/standard_timebase.den;
    
    struct tm *gm = localtime(&epoch);
    
    
    size_t written = (size_t)strftime(buf, K_TS_MAX_STRING_SIZE, shortFormat ? "%H:%M:%S" : "%Y-%m-%dT%H:%M:%S", gm);
    if ((written > 0) && ((size_t)written < K_TS_MAX_STRING_SIZE))
    {
        snprintf(buf+written, K_TS_MAX_STRING_SIZE-(size_t)written, ".%03ld", ((1000*ts) / standard_timebase.den) % 1000);
        
    }
    return buf;
}

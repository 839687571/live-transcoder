//
//  logger.h
//  live-transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef LOGGER_h
#define LOGGER_h

#include <stdbool.h>
#include <libavformat/avformat.h>


#define CATEGORY_DEFAULT "DEFAULT"
#define CATEGORY_CODEC "CODEC"
#define CATEGORY_OUTPUT "OUTPUT"

void logger1(char* category,int level,const char *fmt, ...);
void logger2(char* category,int level,const char *fmt, va_list args);

const char* pict_type_to_string(int pt);

#define K_TS_MAX_STRING_SIZE 100

char *av_ts_make_time_stringEx(char *buf, int64_t ts,bool shortFormat);

/**
 * Convenience macro, the return value should be used only directly in
 * function arguments but never stand-alone.
 */
#define ts2str(ts,short) av_ts_make_time_stringEx((char[K_TS_MAX_STRING_SIZE]){0}, ts,short)

#define LOGGER(CATEGORY,LEVEL,FMT,...) { logger1(CATEGORY,LEVEL,FMT,__VA_ARGS__); }
#define LOGGER0(CATEGORY,LEVEL,FMT) { logger1(CATEGORY,LEVEL,FMT); }

static  AVRational standard_timebase = {1,90000};

uint64_t getTime64();
#endif /* LOGGER_h */

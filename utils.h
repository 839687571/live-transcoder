//
//  utils.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 22/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <stdbool.h>
#include <libavformat/avformat.h>

int load_file_to_memory(const char *filename, char **result);

uint64_t getClock64();
uint64_t getTime64();

#define K_TS_MAX_STRING_SIZE 100

char *av_ts_make_time_stringEx(char *buf, int64_t ts,bool shortFormat);

/**
 * Convenience macro, the return value should be used only directly in
 * function arguments but never stand-alone.
 */
#define ts2str(ts,short) av_ts_make_time_stringEx((char[K_TS_MAX_STRING_SIZE]){0}, ts,short)
static AVRational standard_timebase = {1,90000};
static AVRational clockScale = {1,1000*1000};

#endif /* utils_h */

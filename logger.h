//
//  logger.h
//  live-transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef LOGGER_h
#define LOGGER_h
#include <libavformat/avformat.h>



#define CATEGORY_DEFAULT "DEFAULT"
#define CATEGORY_CODEC "CODEC"
#define CATEGORY_OUTPUT "OUTPUT"

void logger1(char* category,int level,const char *fmt, ...);
void logger2(char* category,int level,const char *fmt, va_list args);

const char* pict_type_to_string(int pt);


#define LOGGER(CATEGORY,LEVEL,FMT,...) { logger1(CATEGORY,LEVEL,FMT,__VA_ARGS__); }
#define LOGGER0(CATEGORY,LEVEL,FMT) { logger1(CATEGORY,LEVEL,FMT); }

#endif /* LOGGER_h */

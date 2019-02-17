//
//  logger.h
//  live-transcoder
//
//  Created by Guy.Jacubovski on 31/12/2018.
//  Copyright © 2018 Kaltura. All rights reserved.
//

#ifndef logger_h
#define logger_h


#define CATEGORY_DEFAULT "DEFAULT"
#define CATEGORY_CODEC "CODEC"
#define CATEGORY_OUTPUT "OUTPUT"

void logger(char* category,int level,const char *fmt, ...);
const char* pict_type_to_string(int pt);


uint64_t getTime64();
#endif /* logger_h */

//
//  fileReader.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 22/03/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef fileReader_h
#define fileReader_h

#include "core.h"
#include "logger.h"
#include "config.h"
#include "utils.h"

int stream_from_file(const char* pSourceFileName,bool *keepRunning);

#endif /* fileReader_h */

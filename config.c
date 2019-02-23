//
//  config.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 23/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "config.h"
#include "logger.h"
#include "json_parser.h"

pool_t *pool;
json_value_t config;

json_value_t* GetConfig()
{
    return &config;
}

int LoadConfig()
{
    
    char* inputConfig;
    load_file_to_memory("/Users/guyjacubovski/dev/live-transcoder/config.json", &inputConfig);
    
    char error[128];
    json_status_t status = json_parse(pool, inputConfig, &config, error, sizeof(error));
    if (status!=JSON_OK) {
        LOGGER(CATEGORY_DEFAULT,AV_LOG_FATAL,"Failed parsing configurtion! %s (%s)",inputConfig,error);
        return -1;
    }
    LOGGER(CATEGORY_DEFAULT,AV_LOG_INFO,"Parsed configuration successfully: %s",inputConfig);
    return 0;
}

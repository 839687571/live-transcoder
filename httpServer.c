//
//  httpServer.c
//  live_transcoder
//
//  Created by Guy.Jacubovski on 22/03/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#include "core.h"
#include "httpServer.h"
#include "utils.h"
#include "logger.h"
#include <pthread.h>
#include "config.h"

pthread_t http_server_thread_id;

pthread_cond_t http_cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t http_lock = PTHREAD_MUTEX_INITIALIZER;

#define CATEGORY_HTTP_SERVER "HttpServer"


static void process_client(AVIOContext *client,http_request_callback callback)
{
    uint8_t buf[10240];
    int ret, n, reply_code;
    uint8_t *resource = NULL;
    while ((ret = avio_handshake(client)) > 0) {
        av_opt_get(client, "resource", AV_OPT_SEARCH_CHILDREN, &resource);
        // check for strlen(resource) is necessary, because av_opt_get()
        // may return empty string.
        if (resource && strlen(resource))
            break;
        
        av_freep(&resource);
    }
    if (ret < 0)
        goto end;
    
    av_log(client, AV_LOG_TRACE, "resource=%p\n", resource);
    
    if (resource && resource[0] == '/') {
        reply_code =  callback(resource,buf,sizeof(buf),&n);
    } else {
        reply_code = AVERROR_HTTP_NOT_FOUND;
    }
    if ((ret = av_opt_set_int(client, "reply_code", reply_code, AV_OPT_SEARCH_CHILDREN)) < 0) {
        av_log(client, AV_LOG_ERROR, "Failed to set reply_code: %s.\n", av_err2str(ret));
        goto end;
    }
    if ((ret = av_opt_set(client, "content_type", "application/json", AV_OPT_SEARCH_CHILDREN)) < 0) {
        av_log(client, AV_LOG_ERROR, "Failed to set reply_code: %s.\n", av_err2str(ret));
        goto end;
    }
    
    
    av_log(client, AV_LOG_TRACE, "Set reply code to %d\n", reply_code);
    
    while ((ret = avio_handshake(client)) > 0);
    
    if (ret < 0)
        goto end;
    
    fprintf(stderr, "Handshake performed.\n");
    if (reply_code != 200)
        goto end;
    
    avio_write(client, buf, n);
end:
    fprintf(stderr, "Flushing client\n");
    avio_flush(client);
    fprintf(stderr, "Closing client\n");
    avio_close(client);
    av_freep(&resource);
}

void* httpServerThread(void *vargp)
{
    http_request_callback callback = (http_request_callback)vargp;

    LOGGER0(CATEGORY_HTTP_SERVER,AV_LOG_INFO,"http listener thread");
    AVIOContext *server = NULL;
    AVDictionary *options = NULL;
    AVIOContext *client = NULL;
    int ret=0;
    
    if ((ret = av_dict_set(&options, "listen", "2", 0)) < 0) {
        LOGGER(CATEGORY_HTTP_SERVER,AV_LOG_FATAL,"http listener failed set listen mode for %d %s",ret,av_err2str(ret));
        return ret;
    }
    if ((ret = avio_open2(&server, "http://localhost:12345", AVIO_FLAG_WRITE, NULL, &options)) < 0) {
        LOGGER(CATEGORY_HTTP_SERVER,AV_LOG_FATAL,"Failed to open server: %d %s",ret,av_err2str(ret));
        return ret;
    }
    
    LOGGER0(CATEGORY_RECEIVER,AV_LOG_INFO,"Waiting for accept");
    
    pthread_cond_signal(&http_cond1);
    
    
    for(;;) {
        if ((ret = avio_accept(server, &client)) < 0) {
            LOGGER(CATEGORY_HTTP_SERVER,AV_LOG_INFO,"Failed to avio_accept: %d (%s)", ret,av_err2str(ret));
            break;
        }
        LOGGER0(CATEGORY_HTTP_SERVER,AV_LOG_INFO,"Accepted client");
        process_client(client,callback);
    }
    
    avio_close(server);
    
    
    LOGGER0(CATEGORY_HTTP_SERVER,AV_LOG_INFO,"Completed receive thread");
    
    return NULL;
}


void start_http_server(int port,http_request_callback callback)
{
    pthread_create(&http_server_thread_id, NULL, httpServerThread,callback);
    pthread_cond_wait(&http_cond1, &http_lock);
    
}


void stop_http_server() {
    
 //    pthread_join(http_server_thread_id,NULL);
}

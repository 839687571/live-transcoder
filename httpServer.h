//
//  httpServer.h
//  live_transcoder
//
//  Created by Guy.Jacubovski on 22/03/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef httpServer_h
#define httpServer_h

#include <stdio.h>

typedef int (*http_request_callback)(const char* uri, char* buf,int bufSize,int* bytesWritten);


void start_http_server(int port,http_request_callback callback);
void stop_http_server();


#endif /* httpServer_h */

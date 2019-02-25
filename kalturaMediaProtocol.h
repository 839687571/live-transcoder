//
//  kalturaMediaProtocol.h
//  live-transcoder
//
//  Created by Guy.Jacubovski on 25/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef kalturaMediaProtocol_h
#define kalturaMediaProtocol_h

typedef struct {
    uint16_t channels;
    uint16_t bits_per_sample;
    uint32_t sample_rate;
} audio_media_info_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    double frame_rate;    // currently rounded by nginx-rtmp, will need a patch to avoid it
} video_media_info_t;


//KMP
typedef struct media_info_s {
    uint32_t media_type;    // 0 = video, 1 = audio
    uint32_t format;    // 4cc code?
    uint32_t timescale;    // currently hardcoded to 90k, maybe for audio we should use the sample rate
    uint32_t bitrate;    // bps    (rtmp module returns in kbps, will multiply by 1000)
    union {
        video_media_info_t video;
        audio_media_info_t audio;
    } u;
    uint32_t extraDataLength;
    char extraData[10000];
} media_info_t;

typedef struct   {
    uint64_t dts;
    uint32_t pts_delay;
    uint32_t size;
    uint32_t flags;
} kaltura_network_frame_t;


#endif /* kalturaMediaProtocol_h */

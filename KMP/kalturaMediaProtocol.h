//
//  kalturaMediaProtocol.h
//  live-transcoder
//
//  Created by Guy.Jacubovski on 25/02/2019.
//  Copyright © 2019 Kaltura. All rights reserved.
//

#ifndef kalturaMediaProtocol_h
#define kalturaMediaProtocol_h
#define PACKET_TYPE_HEADER (1)
#define PACKET_TYPE_FRAME (2)
#define PACKET_TYPE_EOS (0)

typedef struct {
    uint32_t packet_type;
    uint32_t header_size;
    uint32_t data_size;
} packet_header_t;

enum {
    MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_AUDIO,
    MEDIA_TYPE_COUNT,
};

typedef struct {
    uint16_t channels;
    uint16_t bits_per_sample;
    uint32_t sample_rate;
} audio_media_info_t;

typedef struct {
    uint16_t den,num;
} rational_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    rational_t sample_aspect_ratio;
    rational_t frame_rate;        // currently rounded by nginx-rtmp, will need a patch to avoid it
} video_media_info_t;

typedef struct media_info_s {
    packet_header_t header;
    uint32_t media_type;    // 0 = video, 1 = audio
    uint32_t format;        // currently rtmp enum
    uint32_t timescale;        // currently hardcoded to 90k, maybe for audio we should use the sample rate
    uint32_t bitrate;        // bps    (rtmp module returns in kbps, will multiply by 1000)
    union {
        video_media_info_t video;
        audio_media_info_t audio;
    } u;
} media_info_t;

typedef struct {
    packet_header_t header;
    uint32_t flags;
    uint32_t pts_delay;
    uint64_t dts;
} output_frame_t;




#endif /* kalturaMediaProtocol_h */

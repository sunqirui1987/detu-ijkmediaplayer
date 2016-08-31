#ifndef __FRAME_H__
#define __FRAME_H__
#include <stdint.h>
/* Media types. */
typedef enum {
    MediaType_Video = 0,
    MediaType_Audio,
    MediaType_MetaData,
} MediaType;

typedef enum {
    CodecType_H264 = 0,
    CodecType_PCMU,
    CodecType_PCMA,

    CodecType_RawData = 128,
} CodecType;

typedef struct {
	MediaType media;	/* 0 - video, 1 - audio, 2 - metadata */
	CodecType codec;	/* 0 - H.264, 1 - G.711ALaw, 2 - G.711uLaw */

    int64_t pts;
    int64_t time_ms;

    uint8_t keyframe;
    uint8_t extend[3];

    int32_t rawsize;
    uint8_t rawdata[0];
} MediaFrame;

#endif

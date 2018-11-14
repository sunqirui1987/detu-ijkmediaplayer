#ifndef __DECODE_VIDEOSINK_H__
#define __DECODE_VIDEOSINK_H__

#include "viddec.h"

typedef struct VideoSink VideoSink;

int VideoSink_Init(void);
int VideoSink_Quit(void);

VideoSink *VideoSink_OpenStream(char *url);
int VideoSink_CloseStream(VideoSink *sink);
int VideoSink_ReadStream(VideoSink *sink, VideoPkt *pkt, int time_ms);

int VideoSink_HasKeyFrameBuffered(VideoSink *sink);
int VideoSink_NumberFrameBuffered(VideoSink *sink);

#endif

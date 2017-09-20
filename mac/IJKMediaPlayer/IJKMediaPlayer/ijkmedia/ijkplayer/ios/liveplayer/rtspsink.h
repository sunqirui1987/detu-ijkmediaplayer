//
//  rtspsink.h
//  IJKMediaPlayer
//
//  Created by Mac-hcm on 16/8/22.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#ifndef rtspsink_h
#define rtspsink_h

#include <stdio.h>
#include "avqueue.h"
#include "libavformat/avformat.h"

typedef struct RtspSession_t RtspSession_t;

void rtsp_init(void);
RtspSession_t* rtsp_open(const char *url, av_queue *q);
int rtsp_stop(RtspSession_t *s);
int rtsp_read(av_queue *q, AVPacket *pkt);

#endif /* rtspsink_h */

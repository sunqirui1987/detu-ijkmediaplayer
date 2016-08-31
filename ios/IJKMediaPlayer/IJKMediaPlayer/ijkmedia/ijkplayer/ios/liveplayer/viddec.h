/******************************************************************
 * File Name    : decode.c
 * Description  : video decoder api
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-05
 ******************************************************************/

#ifndef __VIDDEC_H__
#define __VIDDEC_H__

#include "decode.h"

typedef struct VideoDec VideoDec;

VideoDec *VideoDec_Create(void);
int VideoDec_Destroy(VideoDec *dec);

typedef struct {
    int width;
    int height;

    int keyframe;

    int64_t pts;

    uint8_t *data;
    int size;
    int pkt_num;
    int refer;
} VideoPkt;

int VideoDec_Decode(VideoDec *dec, VideoPkt *inpkt, VideoFrame *frame);

#endif

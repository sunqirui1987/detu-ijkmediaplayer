/******************************************************************
 * File Name    : decode.c
 * Description  : video decoder
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-04
 ******************************************************************/

#ifndef __DECODE_H__
#define __DECODE_H__
#include <stdint.h>

int Decode_Init(void);
int Decode_Quit(void);

typedef struct DecodeCtx DecodeCtx;

DecodeCtx *Decode_OpenStream(char *url);
int Decode_CloseStream(DecodeCtx *ctx);

typedef struct {
    int width;
    int height;

    long opaque;
    int64_t pts;

    int keyframe;

    uint8_t *data;
    int      size;
} VideoFrame;

int Decode_ReadFrame(DecodeCtx *ctx, VideoFrame *frame);

#endif

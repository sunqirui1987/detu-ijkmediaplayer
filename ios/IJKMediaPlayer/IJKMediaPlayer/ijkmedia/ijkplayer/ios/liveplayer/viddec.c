/******************************************************************
 * File Name    : decode.c
 * Description  : video decoder
 * Author       : huangchengman@detu.com
 * Date         : 2016-07-05
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>

#include "viddec.h"
#include "decode.h"

#ifdef ANDROID_APP
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "VidDec", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "VidDec", format, ##__VA_ARGS__)
#endif

struct VideoDec {
    AVCodec *codec;
    AVFrame *frame;
    AVCodecContext *dec_ctx;

    uint8_t *buffer;
    int      bufsiz;
    int      datsiz;

    int need_keyframe;
};

static int64_t get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

VideoDec *VideoDec_Create(void)
{
    av_register_all();
    avformat_network_init();

    VideoDec *dec = (VideoDec *)malloc(sizeof(VideoDec));
    if (!dec)
        return NULL;
    memset(dec, 0, sizeof(VideoDec));

    dec->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!dec->codec)
        goto fail;

    dec->dec_ctx = avcodec_alloc_context3(dec->codec);

    if (!dec->dec_ctx)
        goto fail;

    if (avcodec_open2(dec->dec_ctx, dec->codec, NULL) < 0)
        goto fail;

    dec->frame = av_frame_alloc();
    if (!dec->frame)
        goto fail;

    dec->need_keyframe = 1;

    return dec;

fail:
    if (dec->frame)
        av_frame_free(&dec->frame);
    if (dec->dec_ctx)
        avcodec_close(dec->dec_ctx);
    if (dec->buffer)
        free(dec->buffer);
    free(dec);

    return NULL;
}

int VideoDec_Destroy(VideoDec *dec)
{
    printf("VideoDec_Destroy########1");
    if (dec->frame){
        printf("free dec frame");
        av_frame_free(&dec->frame);
    }
    if (dec->dec_ctx){
        printf("avcodec_close@#@@@@");
        avcodec_close(dec->dec_ctx);
    }
    if (dec->buffer){
        printf("free dec tmp buffer");
        free(dec->buffer);
    }
    free(dec);

    return 0;
}

static int get_video_frame(VideoDec *dec, AVFrame *frame, VideoFrame *vf)
{
    int bufsize = av_image_get_buffer_size(frame->format, frame->width, frame->height, 1);

    if (!vf->data || vf->opaque < bufsize) {
        if (vf->data)
            free(vf->data);
        vf->data   = malloc(bufsize);
        vf->opaque = bufsize;
    }

    if (!vf->data)
        return -1;

    vf->size = bufsize;

    av_image_copy_to_buffer(vf->data, bufsize, (const uint8_t **)(frame->data),
                frame->linesize, frame->format, frame->width, frame->height, 1);

    vf->width  = frame->width;
    vf->height = frame->height;
    vf->pts    = frame->pkt_pts;

    return 0;
}

int VideoDec_Decode(VideoDec *dec, VideoPkt *inpkt, VideoFrame *frame)
{
    int ret;
    
    AVPacket pkt = {
        .pts   = inpkt->pts,
        .data  = inpkt->data,
        .size  = inpkt->size,
        .flags = inpkt->keyframe ? AV_PKT_FLAG_KEY : 0,
    };
    
    //printf("--->to decode,pkt size:%d\n", inpkt->size);
    int got_frame = 0;

    //av_frame_free(&dec->frame);
    //dec->frame = av_frame_alloc();
    ret = avcodec_decode_video2(dec->dec_ctx, dec->frame, &got_frame, &pkt);
    if ( ret < 0){
        printf("decode failed,ret:%d\n",ret);
        return -1;
    }
    
    int decode_error_flags = av_frame_get_decode_error_flags(dec->frame);
    if (decode_error_flags) {
        printf("error frame, decode_error_flags: 0x%08x\n", dec->frame->decode_error_flags);
        return 0;
    }
   
    if (got_frame) {
        get_video_frame(dec, dec->frame, frame);
        inpkt->refer--;
        //printf("--->decoded pkt width:%d, height:%d \n",frame->width, frame->height);
        return 1;
    }

    return 0;
}

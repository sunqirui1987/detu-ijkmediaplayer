/******************************************************************
 * File Name    : decode.c
 * Description  : video decoder
 * Author       : huangchengman@detu.com
 * Date         : 2016-07-05
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "queue.h"
#include "decode.h"
#include "viddec.h"
#include "videosink.h"

#include "rtspsink.h"
#include "avqueue.h"

#include <libavformat/avformat.h>

#define BUFFERED_RAWFRAME_NUMBER   (32)
#define BUFFERED_YUVFRAME_NUMBER   (2)

#ifdef ANDROID_APP
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "VideoDec", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "VideoDec", format, ##__VA_ARGS__)
#endif

struct DecodeCtx {
    char url[1024];

    int64_t pts_base;
    int64_t timebase;

    Queue *fullq;
    Queue *emptyq;

    //VideoSink *videosink;
    RtspSession_t *sess;
    VideoDec *viddec;
    
    av_queue *q;

    pthread_t decode_tid;

    int abort;

    uint8_t *buffer;
    uint8_t bufsiz;
};

static int64_t get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

static VideoFrame *obtain_empty_frame(DecodeCtx *ctx)
{
    VideoFrame *vf = NULL;

    int i;
    for (i = 0; i < 2; i++) {
        vf = queue_remove(ctx->emptyq, 10);
        if (vf)
            return vf;

        vf = queue_remove(ctx->fullq, 0);
        if (vf) {
            return vf;
        }
    }

    return NULL;
}

static void *video_decode_thread(void *arg)
{
    DecodeCtx *ctx = (DecodeCtx *)arg;

    int need_keyframe = 1;
    
    while (!ctx->abort) {
        VideoPkt inpkt;
        AVPacket pkt;
        
        int ret = rtsp_read(ctx->q, &pkt);
//        int ret = VideoSink_ReadStream(ctx->videosink, &inpkt, 100);
        if (ret < 0)
            break;
        else if (ret == 0)
            continue;

        inpkt.data = pkt.data;
        inpkt.keyframe = pkt.flags;
        inpkt.size = pkt.size;
        inpkt.pts = pkt.pts;
//        printf("new rtsp read pkt,pkt size:%d,pkt pts:%lld\n",pkt.size, pkt.pts);
//        
//        if (need_keyframe && !inpkt.keyframe)
//            continue;
//
//        if (inpkt.keyframe)
//            need_keyframe = 0;

        VideoFrame *frame = obtain_empty_frame(ctx);
        if (!frame)
            continue;

        if (VideoDec_Decode(ctx->viddec, &inpkt, frame) <= 0) {
            queue_insert(ctx->emptyq, frame, 100);
#ifdef ANDORID_APP
	    LOGE("decode failed! \n");
#endif
            continue;
        }
        av_free_packet(&pkt);
        queue_insert(ctx->fullq, frame, 100);
    }

    return 0;
}

int Decode_Init(void)
{
    //VideoSink_Init();
    rtsp_init();

    return 0;
}

int Decode_Quit(void)
{
    //VideoSink_Quit();

    return 0;
}

static int close_stream(DecodeCtx *ctx)
{
    printf("close_stream*********1\n");
    ctx->abort = 1;

    if (ctx->decode_tid)
        pthread_join(ctx->decode_tid, NULL);
    
    rtsp_stop(ctx->sess);
    queue_flush(ctx->q);
   // VideoSink_CloseStream(ctx->videosink);
    VideoDec_Destroy(ctx->viddec);
    if (ctx->fullq) {
        VideoFrame *vf;
        while ((vf = queue_remove(ctx->fullq, 0)) != NULL) {
            if (vf->data)
                free(vf->data);
            free(vf);
        }
        queue_destroy(ctx->fullq);
    }

    if (ctx->emptyq) {
        VideoFrame *vf;
        while ((vf = queue_remove(ctx->emptyq, 0)) != NULL) {
            if (vf->data)
                free(vf->data);
            free(vf);
        }
        queue_destroy(ctx->emptyq);
    }

    if (ctx->buffer)
        free(ctx->buffer);

    free(ctx);

    return 0;
}

static int is_localhost(char *url)
{
    if (strstr(url, "rtsp://127.0.0.1"))
        return 1;
    if (strstr(url, "rtsp://localhost"))
        return 1;
    return 0;
}

DecodeCtx *Decode_OpenStream(char *url)
{
    DecodeCtx *ctx = (DecodeCtx *)malloc(sizeof(DecodeCtx));
    if (!ctx)
        return NULL;
    memset(ctx, 0, sizeof(DecodeCtx));

    ctx->q = malloc(sizeof(av_queue));
    queue_init(ctx->q);
    
    ctx->sess = rtsp_open(url, ctx->q);
    
//    ctx->videosink = VideoSink_OpenStream(url);
//    if (!ctx->videosink)
//        goto fail;

    ctx->viddec = VideoDec_Create();
    if (!ctx->viddec)
        goto fail;

    ctx->fullq  = queue_create(BUFFERED_YUVFRAME_NUMBER);
    if (!ctx->fullq)
        goto fail;

    ctx->emptyq = queue_create(BUFFERED_YUVFRAME_NUMBER);
    if (!ctx->emptyq)
        goto fail;

    int i;
    for (i = 0; i < BUFFERED_YUVFRAME_NUMBER; i++) {
        VideoFrame *vf = malloc(sizeof(VideoFrame));
        if (!vf)
            goto fail;
        memset(vf, 0, sizeof(VideoFrame));

        queue_insert(ctx->emptyq, vf, 100);
    }

    ctx->pts_base = -1;
    ctx->timebase = -1;

    strncpy(ctx->url, url, sizeof(ctx->url) - 1);

    pthread_create(&ctx->decode_tid, NULL, video_decode_thread, ctx);

    return ctx;

fail:
    close_stream(ctx);
    return NULL;
}

static void *close_proxy(void *arg)
{
    pthread_detach(pthread_self());

    DecodeCtx *ctx = (DecodeCtx *)arg;
    if (!ctx)
        return NULL;

    close_stream(ctx);

    return NULL;
}

int Decode_CloseStream(DecodeCtx *ctx)
{
    pthread_t tid;
    printf("decode_closestream");
    pthread_create(&tid, NULL, close_proxy, ctx);
    return 0;
}

static int64_t needwait(DecodeCtx *ctx, int64_t pts)
{
    if (ctx->pts_base == -1 || ctx->pts_base > pts) {
        ctx->pts_base = pts;
        ctx->timebase = get_time();
    }

    double itv_pts = pts - ctx->pts_base;
    int64_t next_t = itv_pts / 90000 * 1000 + ctx->timebase;

    int64_t time_ms = next_t - get_time();
    if (time_ms < 50 && time_ms >= 0)
        return time_ms;

    ctx->pts_base = pts;
    ctx->timebase = get_time();

    return 0;
}

int Decode_ReadFrame(DecodeCtx *ctx, VideoFrame *frame)
{
    VideoFrame *vf = queue_remove(ctx->fullq, 1000);
    if (!vf){
	//LOGE("no decoded frame!\n");
        return 0;
    }

    if (!ctx->buffer || ctx->bufsiz < vf->size) {
        if (ctx->buffer)
            free(ctx->buffer);
        ctx->buffer = malloc(vf->size);
        ctx->bufsiz = vf->size;
    }

    memcpy(ctx->buffer, vf->data, vf->size);

    memset(frame, 0, sizeof(VideoFrame));
    frame->width  = vf->width;
    frame->height = vf->height;
    frame->pts    = vf->pts;
    frame->data   = ctx->buffer;
    frame->size   = vf->size;

    queue_insert(ctx->emptyq, vf, 1000);

    int time_ms = (int)needwait(ctx, frame->pts);
    if (time_ms > 0 && time_ms < 30){
        usleep(time_ms * 1000);
    }else if (time_ms > 30) {
        usleep((time_ms - 30) * 1000);
    }

    return 1;
}

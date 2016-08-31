//
//  rtspsink.c
//  IJKMediaPlayer
//
//  Created by Mac-hcm on 16/8/22.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <libavformat/avformat.h>

#include "rtspsink.h"
#include "avqueue.h"

typedef struct RtspSession_t {
    char url[1024];
    int refcnt;
    
    pthread_t stream_tid;
    int abort;
    av_queue *q;
} RtspSession_t;

int rtsp_service(RtspSession_t *s)
{
    printf("rtsp service start##############,url:%s\n",s->url);
    
    int video_stream_idx = -1;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *dec_ctx = NULL;
    
    //force rtsp by tcp transport
    AVDictionary *option = NULL;
//    av_dict_set(&option, "rtsp_transport", "tcp", 0);
//    av_dict_set(&option, "timeout", "30*1000*1000", 0);
//    av_dict_set(&option, "stimeout", "30", 0);
    
    int ret;
    if (avformat_open_input(&fmt_ctx, s->url, NULL, &option) < 0)
        goto fail;
    
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
        goto fail;
    
    video_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_idx < 0)
        goto fail;
    
    dec_ctx = fmt_ctx->streams[video_stream_idx]->codec;
    while (!s->abort) {
        AVPacket pkt = {0};
        av_init_packet(&pkt);
        ret = av_read_frame(fmt_ctx, &pkt);
        if (ret < 0) {
            printf("av_read_frame failed!!!,ret:%d",ret);
            break;
        }
        
        if (pkt.stream_index != video_stream_idx) {  /* video only */
            av_free_packet(&pkt);
            continue;
        }
        av_dup_packet(&pkt);
        
        ret = put_queue(s->q, &pkt);
        if (ret < 0) {
            av_free_packet(&pkt);
        }
        //printf("===>rtsp service get video pkt,pkt.size:%d,pkt.pts:%lld\n",pkt.size, pkt.pts);
    }
    
fail:
    if (fmt_ctx)
        avformat_close_input(&fmt_ctx);
    
    return 0;
}

void *video_stream_thread(void *arg)
{
    RtspSession_t *s = (RtspSession_t *)arg;
    
    while (!s->abort) {
        rtsp_service(s);
        usleep(200);
    }
    
    return NULL;
}

void rtsp_init(void)
{
    av_register_all();
    avformat_network_init();
}

RtspSession_t* rtsp_open(const char *url, av_queue *q)
{
    RtspSession_t *s;
    s = malloc(sizeof(RtspSession_t));
    if (!s)
    {
        printf("create rtspsession failed\n");
        return NULL;
    }
    
    s->abort = 0;
    printf("rtsp open url:%s\n",url);
    strncpy(s->url, url, strlen(url) + 1);
    s->q = q;
    q->m_type = QUEUE_PACKET;
    pthread_create(&s->stream_tid, NULL, video_stream_thread, s);
    
    return s;
}

int rtsp_stop(RtspSession_t *s)
{
    printf("rtsp session stop\n");
    s->abort = 1;
    pthread_join(s->stream_tid, NULL);
    if (!s)
    {
        free(s);
        s = NULL;
    }
    
    return 0;
}

int rtsp_read(av_queue *q, AVPacket *pkt)
{
    int ret;
    q->m_type = QUEUE_PACKET;
    ret = get_queue(q, pkt);
    if (ret < 0)
    {
        printf("rtsp read packet failed\n");
        return ret;
    }
    
    return ret;
}

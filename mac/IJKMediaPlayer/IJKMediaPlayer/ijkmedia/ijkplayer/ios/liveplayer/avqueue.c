//
//  avqueue.c
//  IJKMediaPlayer
//
//  Created by huangchengman on 16/8/20.
//  Copyright © 2016年 detu net company. All rights reserved.
//

#include "avqueue.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include "libavformat/avformat.h"

static AVPacket flush_pkt;
static AVFrame flush_frm;

typedef struct AVFrameList
{
    AVFrame pkt;
    struct AVFrameList *next;
}AVFrameList;


void queue_init(av_queue *q)
{
    q->abort_request = 0;
    q->m_first_pkt = q->m_last_pkt = NULL;
    q->m_size = 0;
    
    pthread_mutex_init(&q->m_mutex, NULL);
    pthread_cond_init(&q->m_cond, NULL);
    
    if (q->m_type == QUEUE_PACKET)
    {
        put_queue(q, (void *)&flush_pkt);
    }
    else if (q->m_type == QUEUE_AVFRAME)
    {
        put_queue(q, (void *)&flush_frm);
    }
}

void queue_flush(av_queue *q)
{
    if (q->m_size == 0)
        return;
    
    if (q->m_type == QUEUE_PACKET)
    {
        AVPacketList *pkt, *pkt1;
        pthread_mutex_lock(&q->m_mutex);
        for (pkt = q->m_first_pkt; pkt != NULL; pkt = pkt1)
        {
            pkt1 = pkt->next;
            if (pkt->pkt.data != flush_pkt.data)
                av_free_packet(&pkt->pkt);
            av_freep(&pkt);
        }
        q->m_last_pkt = NULL;
        q->m_first_pkt = NULL;
        q->m_size = 0;
        pthread_mutex_unlock(&q->m_mutex);
    }
    else if (q->m_type == QUEUE_AVFRAME)
    {
        AVFrameList *pkt, *pkt1;
        pthread_mutex_lock(&q->m_mutex);
        for (pkt = q->m_first_pkt; pkt != NULL; pkt = pkt1)
        {
            pkt1 = pkt->next;
            if (pkt->pkt.data[0] != flush_frm.data[0])
                av_free(pkt->pkt.data[0]);
            av_freep(&pkt);
        }
        q->m_last_pkt = NULL;
        q->m_first_pkt = NULL;
        q->m_size = 0;
        pthread_mutex_unlock(&q->m_mutex);
    }
}

void queue_end(av_queue *q)
{
    queue_flush(q);
#ifdef WIN32
    if (q->m_cond)
        pthread_cond_destroy(&q->m_cond);
    if (q->m_mutex)
        pthread_mutex_destroy(&q->m_mutex);
#else
    pthread_cond_destroy(&q->m_cond);
    pthread_mutex_destroy(&q->m_mutex);
#endif
}

#define PRIV_OUT_QUEUE \
        pthread_mutex_lock(&q->m_mutex); \
            for (;;) \
            { \
                if (q->abort_request) \
                { \
                    ret = -1; \
                    break; \
                } \
                pkt1 = q->m_first_pkt; \
                if (pkt1) \
                { \
                    q->m_first_pkt = pkt1->next; \
                    if (!q->m_first_pkt) \
                    q->m_last_pkt = NULL; \
                    q->m_size--; \
                    *pkt = pkt1->pkt; \
                    av_free(pkt1); \
                    ret = 1; \
                    break; \
                } \
                else \
                { \
                    pthread_cond_wait(&q->m_cond, &q->m_mutex); \
                } \
            } \
        pthread_mutex_unlock(&q->m_mutex);

int get_queue(av_queue *q, void *p)
{
    if (q->m_type == QUEUE_PACKET)
    {
        AVPacketList *pkt1;
        AVPacket *pkt = (AVPacket*) p;
        int ret;
        PRIV_OUT_QUEUE
        return ret;
    }
    else if (q->m_type == QUEUE_AVFRAME)
    {
        AVFrameList *pkt1;
        AVFrame *pkt = (AVFrame*) p;
        int ret;
        PRIV_OUT_QUEUE
        return ret;
    }
    return -1;
}

#define PRIV_PUT_QUEUE(type) \
    pkt1 = av_malloc(sizeof(type)); \
    if (!pkt1) \
        return -1; \
    pkt1->pkt = *pkt; \
    pkt1->next = NULL; \
    \
    pthread_mutex_lock(&q->m_mutex); \
    if (!q->m_last_pkt) \
        q->m_first_pkt = pkt1; \
    else \
        ((type*) q->m_last_pkt)->next = pkt1; \
    q->m_last_pkt = pkt1; \
    q->m_size++; \
    pthread_cond_signal(&q->m_cond); \
    pthread_mutex_unlock(&q->m_mutex);

int put_queue(av_queue *q, void *p)
{
    if (q->m_type == QUEUE_PACKET)
    {
        AVPacketList *pkt1;
        AVPacket *pkt = (AVPacket*) p;
        /* duplicate the packet */
        if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
            return -1;
        PRIV_PUT_QUEUE(AVPacketList)
        return 0;
    }
    else if (q->m_type == QUEUE_AVFRAME)
    {
        AVFrameList *pkt1;
        AVFrame *pkt = (AVFrame*) p;
        PRIV_PUT_QUEUE(AVFrameList)
        return 0;
    }
    
    return -1;
}

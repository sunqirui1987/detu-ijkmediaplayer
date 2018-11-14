//
//  avqueue.h
//  IJKMediaPlayer
//
//  Created by huangchengman on 16/8/20.
//  Copyright © 2016年 detu net company. All rights reserved.
//

#ifndef avqueue_h
#define avqueue_h

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#define QUEUE_PACKET                (0)
#define QUEUE_AVFRAME               (1)

enum bool_type
{
    FALSE, TRUE
};

typedef struct _av_queue
{
    void *m_first_pkt, *m_last_pkt;
    int m_size;
    int m_type; //pkt queue type(video or audio)
    int abort_request;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
}av_queue;

void queue_init(av_queue *q);
void queue_flush(av_queue *q);
void queue_end(av_queue *q);

int get_queue(av_queue *q, void *p);
int put_queue(av_queue *q, void *p);

#endif /* avqueue_h */

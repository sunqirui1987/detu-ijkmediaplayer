/******************************************************************
 * File Name    : queue.c
 * Description  : simple queue implemention
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-06
 ******************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "queue.h"

typedef struct QueueNode {
    void *data;
} QueueNode;

struct Queue {
    int max_entities;

    int64_t widx;
    int64_t ridx;

    QueueNode *nodes;

    pthread_mutex_t lock;
    pthread_cond_t r_cond;
    pthread_cond_t w_cond;
};

static struct timespec time_after(int milliseconds)
{
	struct timespec tp;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	tp.tv_sec = tv.tv_sec;
	tp.tv_nsec = tv.tv_usec * 1000;

	int64_t nsec = tp.tv_nsec + (int64_t)milliseconds * 1000000;

	tp.tv_sec  += nsec / 1000000000;
	tp.tv_nsec  = nsec % 1000000000;

	return tp;
}

Queue *queue_create(int max_entities)
{
    Queue *q = malloc(sizeof(Queue));
    if (!q)
        return NULL;
    memset(q, 0, sizeof(Queue));

    q->max_entities = max_entities;
    q->nodes = malloc(sizeof(QueueNode) * max_entities);
    if (!q->nodes) {
        free(q);
        return NULL;
    }

    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->w_cond, NULL);
    pthread_cond_init(&q->r_cond, NULL);

    return q;
}

int queue_destroy(Queue *q)
{
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->r_cond);
    pthread_cond_destroy(&q->w_cond);

    if (q->nodes)
        free(q->nodes);
    free(q);
    return 0;
}

int queue_length(Queue *q)
{
    return (int)(q->widx - q->ridx);
}

static int queue_full(Queue *q)
{
    return queue_length(q) == q->max_entities;
}

static int queue_empty(Queue *q)
{
    return q->widx == q->ridx;
}

int queue_insert(Queue *q, void *data, int milliseconds)
{
    pthread_mutex_lock(&q->lock);

    if (!milliseconds && queue_full(q)) {
        pthread_mutex_unlock(&q->lock);
        return 0;
    } else {
        struct timespec tp = time_after(milliseconds);

        while (queue_full(q)) {
            int ret = pthread_cond_timedwait(&q->w_cond, &q->lock, &tp);
            if (ret != 0) {
                pthread_mutex_unlock(&q->lock);
                return 0;
            }
        }
    }

    QueueNode *n = &q->nodes[q->widx % q->max_entities];

    n->data = data;
    q->widx++;

    pthread_cond_signal(&q->r_cond);

    pthread_mutex_unlock(&q->lock);

    return 1;
}

void *queue_remove(Queue *q, int milliseconds)
{
    void *data = NULL;

    pthread_mutex_lock(&q->lock);

    if (!milliseconds && queue_empty(q)) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    } else {
        struct timespec tp = time_after(milliseconds);

        while (queue_empty(q)) {
            int ret = pthread_cond_timedwait(&q->r_cond, &q->lock, &tp);
            if (ret != 0) {
                pthread_mutex_unlock(&q->lock);
                return NULL;
            }
        }
    }

    QueueNode *n = &q->nodes[q->ridx % q->max_entities];
    q->ridx++;

    data = n->data;

    pthread_cond_signal(&q->w_cond);

    pthread_mutex_unlock(&q->lock);

    return data;
}

#if 0
static void *writer(void *arg)
{
    Queue *q = arg;

    uint8_t i = 0;
    while (1) {
        uint8_t *data = malloc(32*1024);
        memset(data, i++, 32);
        if (queue_insert(q, data, 1) < 0)
            break;
        usleep(100);
    }

    return NULL;
}

static void *reader(void *arg)
{
    Queue *q = arg;

    while (1) {
        uint8_t *data = queue_remove(q, 0);
        if (!data) {
            usleep(100);
            continue;
        }

        printf("--> ");
        int i;
        for (i = 0; i < 32; i++) {
            printf("%02x ", data[i]);
        }
        free(data);
        printf("free data buffer\n");
    }

    return NULL;
}

int main(void)
{
    Queue *q = queue_create(32);

    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, writer, q);
    pthread_create(&tid2, NULL, reader, q);

    sleep(20);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}
#endif

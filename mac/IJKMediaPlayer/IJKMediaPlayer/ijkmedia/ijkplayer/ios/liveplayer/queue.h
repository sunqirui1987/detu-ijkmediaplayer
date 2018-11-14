/******************************************************************
 * File Name    : queue.h
 * Description  : simple queue implemention
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-04
 ******************************************************************/

#ifndef __DECODE_QUEUE_H__
#define __DECODE_QUEUE_H__

typedef struct Queue Queue;

Queue *queue_create(int max_entities);
int queue_destroy(Queue *q);

int queue_insert(Queue *q, void *data, int block);
void *queue_remove(Queue *q, int block);

int queue_length(Queue *q);

#endif

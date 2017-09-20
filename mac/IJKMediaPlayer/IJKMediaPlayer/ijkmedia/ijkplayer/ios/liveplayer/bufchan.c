/******************************************************************
 * File Name    : bufchan.c
 * Description  : audio/video buffer pool implementation
 * Author       : huangchengman@detu.com
 * Date         : 2016-07-04
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/uio.h>
#include <sys/time.h>

#include "fifo.h"
#include "bufchan.h"

#define AV_STREAM_WRITER    (1)
#define AV_STREAM_READER    (2)

typedef struct AVPool {
    char name[128];

    AVFifo *fifo;

    pthread_cond_t  cond;
    pthread_mutex_t lock;

    struct AVPool *next;
} AVPool;

static AVPool *first_pool = NULL;
static pthread_mutex_t lockpool = PTHREAD_MUTEX_INITIALIZER;

typedef struct AVPoolCtx {
    int type;    /* XXX: never move this member */
    AVPool *pool;

    AVFrameIter iter;   /* reader only */
} AVPoolCtx;

static AVPool *av_pool_find_internal(const char *name)
{
    AVPool *p;

    for (p = first_pool; p; p = p->next) {
        if (!strncmp(p->name, name, sizeof(p->name)))
            return p;
    }

    return NULL;
}

static AVPool *av_pool_find(const char *name)
{
    pthread_mutex_lock(&lockpool);
    AVPool *p = av_pool_find_internal(name);
    pthread_mutex_unlock(&lockpool);

    return p;
}

/**
 * buffer pool relative wrapper function
 */
int BufChan_MkFifo(char *name, size_t size)
{
    if (!name || !strlen(name) || !size)
        return -1;

    pthread_mutex_lock(&lockpool);

    if (av_pool_find_internal(name) < 0) {
        pthread_mutex_unlock(&lockpool);
        return -1;    /* allready existed */
    }

    AVPool *p = malloc(sizeof(AVPool));
    if (!p) {
        pthread_mutex_unlock(&lockpool);
        return -1;
    }
    memset(p, 0, sizeof(AVPool));

    p->fifo = av_fifo_create(size);
    if (!p->fifo) {
        free(p);
        return -1;
    }

    pthread_cond_init(&p->cond, NULL);
    pthread_mutex_init(&p->lock, NULL);

    strncpy(p->name, name, sizeof(p->name));

    /* add pool to list */
    p->next = first_pool;
    first_pool = p;

    pthread_mutex_unlock(&lockpool);

    return 0;
}

int BufChan_UnLink(char *name)
{
    if (!name || !strlen(name))
        return -1;

    pthread_mutex_lock(&lockpool);

    AVPool *p1, *p, **pp;

    p = av_pool_find_internal(name);
    if (!p) {
        pthread_mutex_unlock(&lockpool);
        return -1;
    }

    /* remove pool from list */
    pp = &first_pool;
    while ((*pp) != NULL) {
        p1 = *pp;
        if (p1 != p)
            pp = &p1->next;
        else {
            *pp = p->next;
            break;
        }
    }

    pthread_mutex_unlock(&lockpool);

    av_fifo_destroy(p->fifo);

    pthread_cond_destroy(&p->cond);
    pthread_mutex_destroy(&p->lock);

    free(p);

    return 0;
}

long BufChan_Open(char *name, int mode)
{
    if (!name || !name[0])
        return 0;

    AVPoolCtx *ctx = malloc(sizeof(AVPoolCtx));
    if (!ctx)
        return (long)NULL;
    memset(ctx, 0, sizeof(AVPoolCtx));

    ctx->pool = av_pool_find(name);
    if (!ctx->pool) {
        free(ctx);
        return (long)NULL;
    }

    switch (mode) {
    case (AV_RDONLY | AV_CACHED):
        ctx->type = AV_STREAM_READER;
        ctx->iter = av_fifo_next_frame(ctx->pool->fifo, 1);
        break;
    case AV_RDONLY:
        ctx->type = AV_STREAM_READER;
        ctx->iter = av_fifo_next_frame(ctx->pool->fifo, 0);
        break;
    case AV_WRONLY:
        ctx->type = AV_STREAM_WRITER;
        break;
    }

    return (long)ctx;
}

int BufChan_Close(long handle)
{
    AVPoolCtx *ctx = (AVPoolCtx *)handle;
    if (!ctx)
        return -1;

    free(ctx);

    return 0;
}

int BufChan_Reset(long handle)
{
    AVPoolCtx *ctx = (void *)handle;
    if (!ctx)
        return -1;

    AVPool *p = ctx->pool;

    int ret = -1;

    pthread_mutex_lock(&p->lock);

    switch (ctx->type) {
    case AV_STREAM_WRITER:
        ret = av_fifo_erase(p->fifo);
        break;
    case AV_STREAM_READER:
        ret = av_fifo_last(p->fifo, &ctx->iter);
        break;
    }

    pthread_mutex_unlock(&p->lock);

    return ret;
}

int BufChan_Left(long handle)
{
    AVPoolCtx *ctx = (void *)handle;
    if (!ctx)
        return -1;

    AVPool *p = ctx->pool;
    int ret = -1;

    pthread_mutex_lock(&p->lock);

    switch (ctx->type) {
    case AV_STREAM_WRITER:
        ret = av_fifo_length(p->fifo);
        break;
    case AV_STREAM_READER:
        ret = av_fifo_left(p->fifo, &ctx->iter);
        break;
    }

    pthread_mutex_unlock(&p->lock);

    return ret;
}

#define VALID_WRITER(ctx)    (ctx && (ctx->type == AV_STREAM_WRITER) && ctx->pool)
#define VALID_READER(ctx)    (ctx && (ctx->type == AV_STREAM_READER) && ctx->pool)

ssize_t BufChan_WriteV(long handle, struct iovec *iov, size_t iovcnt)
{
    AVPoolCtx *ctx = (AVPoolCtx *)handle;
    if (!VALID_WRITER(ctx) || !iov || !iovcnt)
        return -1;

    AVPool *p = ctx->pool;

    pthread_mutex_lock(&p->lock);

    ssize_t len = av_fifo_writev(p->fifo, iov, (int)iovcnt);
    if (len <= 0) {
        pthread_mutex_unlock(&p->lock);
        return len;
    }

    pthread_cond_broadcast(&p->cond);

    pthread_mutex_unlock(&p->lock);
    return len;
}

ssize_t BufChan_Write(long handle, void *buf, size_t nbyte)
{
    printf("*****BufChan_Write 1");
    if (!handle || !buf || !nbyte)
        return -1;

    struct iovec iov = {
        .iov_base = buf,
        .iov_len  = nbyte,
    };
    return BufChan_WriteV(handle, &iov, 1);
}

inline static int wait_readable(AVPool *p, struct timespec *tp)
{
    int ret;
again:
    ret = pthread_cond_timedwait(&p->cond, &p->lock, tp);
    if (ret == ETIMEDOUT)
        return 0;
    else if (ret == EINTR)
        goto again;

    return 1;
}

#define NSEC_PER_SEC    (1000 * 1000 * 1000)

static int time_after(struct timespec *tp, int milliseconds)
{
#if 0
    clock_gettime(CLOCK_REALTIME, tp);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tp->tv_sec  = tv.tv_sec;
    tp->tv_nsec = tv.tv_usec * 1000;
#endif

    tp->tv_sec  += milliseconds / 1000;
    tp->tv_nsec += milliseconds % 1000 * 1000000;

    if (tp->tv_nsec >= NSEC_PER_SEC) {
        tp->tv_nsec -= NSEC_PER_SEC;
        tp->tv_sec  += 1;
    }

    return 0;
}

ssize_t BufChan_ReadV(long handle, struct iovec *iov, size_t iovcnt, int milliseconds)
{
    AVPoolCtx *ctx = (AVPoolCtx *)handle;
    if (!VALID_READER(ctx) || !iov || !iovcnt) {
        return -1;
    }

    struct timespec timeout, *tp = NULL;

    if (milliseconds > 0) {
        time_after(&timeout, milliseconds);
        tp = &timeout;
    }

    ssize_t len;
    AVPool *p = ctx->pool;

    pthread_mutex_lock(&p->lock);

    while ((len = av_fifo_readv(p->fifo, iov, iovcnt, &ctx->iter)) == 0) {
        if (!tp || !wait_readable(p, tp)) {
            pthread_mutex_unlock(&p->lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&p->lock);

    return len;
}

ssize_t BufChan_Read(long handle, void *buf, size_t nbyte, int milliseconds)
{
    if (!handle || !buf || !nbyte) {
        return -1;
    }
    struct iovec iov = {
        .iov_base = buf,
        .iov_len  = nbyte,
    };
    return BufChan_ReadV(handle, &iov, 1, milliseconds);
}

int BufChan_ForEach(long handle, int (*cb)(struct iovec *iovs, int iovcnt, void *priv_data), void *priv_data)
{
    AVPoolCtx *ctx = (void *)handle;
    if (!ctx)
        return -1;

    AVFrameIter iter;
    AVPool *p = ctx->pool;

    pthread_mutex_lock(&p->lock);

    switch (ctx->type) {
    case AV_STREAM_WRITER:
        iter = av_fifo_first_frame(p->fifo, 0);
        break;
    case AV_STREAM_READER:
        iter = ctx->iter;
        break;
    default:
        pthread_mutex_unlock(&p->lock);
        return -1;
    }

    int ret = av_fifo_foreach(p->fifo, &iter, cb, priv_data);

    pthread_mutex_unlock(&p->lock);

    return ret;
}

ssize_t BufChan_WriteBlock(long handle, size_t maxsize, ssize_t (*write_cb)(struct iovec *iovs, int iovcnt, void *priv_data), void *priv_data)
{
    AVPoolCtx *ctx = (AVPoolCtx *)handle;
    if (!VALID_WRITER(ctx) || !write_cb)
        return -1;

    AVPool *p = ctx->pool;

    pthread_mutex_lock(&p->lock);

    ssize_t len = av_fifo_write_generic(p->fifo, maxsize, write_cb, priv_data);
    if (len <= 0) {
        pthread_mutex_unlock(&p->lock);
        return len;
    }

    pthread_cond_broadcast(&p->cond);

    pthread_mutex_unlock(&p->lock);

    return len;
}

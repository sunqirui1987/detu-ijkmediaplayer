#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>

#include "fifo.h"

#define min(x, y)    ((x) < (y) ? (x) : (y))

struct AVFifo {
    uint32_t wndx;
    uint32_t rndx;

    uint8_t *wptr;        /* write pointer */
    uint8_t *rptr;        /* read pointer  */

    uint32_t first;        /* first index */
    uint32_t next;        /* next index  */

    int      bufsiz;    /* total size */
    uint8_t *bufend;
    uint8_t  buffer[0];
};

typedef struct AVFrame {
    uint32_t size;    /* data size only */
} AVFrame;

inline static void *seek_addr(AVFifo *fifo, uint8_t *ptr, size_t offset)
{
    ptr += offset;
    if (ptr >= fifo->bufend)
        ptr = fifo->buffer + (ptr - fifo->bufend);
    return ptr;
}

AVFifo *av_fifo_create(int size)
{
    int total = sizeof(AVFifo) + size;

    AVFifo *fifo = malloc(total);
    if (!fifo)
        return NULL;
    memset(fifo, 0, sizeof(AVFifo));

    fifo->bufsiz = size;
    fifo->bufend = fifo->buffer + size;

    fifo->first = fifo->next = 0;
    fifo->wptr  = fifo->rptr = fifo->buffer;
    fifo->wndx  = fifo->rndx = 0;

    return fifo;
}

int av_fifo_destroy(AVFifo *fifo)
{
    if (fifo)
        free(fifo);
    return 0;
}

inline static int iovec_size(const struct iovec *iov, int iovcnt)
{
    int len = 0;

    int i;
    for (i = 0; i < iovcnt; i++)
        len += iov[i].iov_len;

    return len;
}

inline static int av_fifo_size(AVFifo *fifo)
{
    return (uint32_t)(fifo->wndx - fifo->rndx);
}

inline static int av_fifo_space(AVFifo *fifo)
{
    return fifo->bufend - fifo->buffer - av_fifo_size(fifo);
}

inline static int valid_index(AVFifo *fifo, int index)
{
    return ((index >= fifo->first) && (index <= fifo->next));
}

int av_fifo_left(AVFifo *fifo, AVFrameIter *iter)
{
    if (!valid_index(fifo, iter->index)) {
        /* invalid frame index, point it to the first frame */
        iter->index = iter->cached ? fifo->first : fifo->next;
        iter->frame = iter->cached ? fifo->rptr  : fifo->wptr;
    }

    if (iter->index == fifo->next)
        return 0;    /* no more frame */

    return fifo->next - iter->index;
}

AVFrameIter av_fifo_next_frame(AVFifo *fifo, int cached)
{
    AVFrameIter iter;
    memset(&iter, 0, sizeof(AVFrameIter));

    iter.cached = cached;
    iter.index = fifo->next;
    iter.frame = fifo->wptr;

    return iter;
}

AVFrameIter av_fifo_first_frame(AVFifo *fifo, int cached)
{
    AVFrameIter iter;
    memset(&iter, 0, sizeof(AVFrameIter));

    iter.cached = cached;
    iter.index = fifo->first;
    iter.frame = fifo->rptr;

    return iter;
}

static ssize_t av_fifo_read_internal(AVFifo *fifo, uint8_t *dst, uint8_t *src, size_t size)
{
    int left = size;

    do {
        int len = min(fifo->bufend - src, left);
        memcpy(dst, src, len);
        dst += len;
        src += len;
        if (src >= fifo->bufend)
            src = fifo->buffer;
        left -= len;
    } while (left > 0);

    return size;
}

ssize_t av_fifo_readv(AVFifo *fifo, struct iovec *iov, int iovcnt, AVFrameIter *iter)
{
    if (av_fifo_left(fifo, iter) == 0)
        return 0;

    /* read frame head */
    AVFrame fr;
    av_fifo_read_internal(fifo, (uint8_t *)&fr, iter->frame, sizeof(AVFrame));

    /* read frame body */
    int space = iovec_size(iov, iovcnt);
    if (space < fr.size)
        return -1;

    iter->frame = seek_addr(fifo, iter->frame, sizeof(AVFrame));

    int i, left, size;
    for (i = 0, left = fr.size; i < iovcnt && left > 0; i++, left -= size) {
        size = min(left, iov[i].iov_len);
        av_fifo_read_internal(fifo, iov[i].iov_base, iter->frame, size);
        iter->frame = seek_addr(fifo, iter->frame, size);
    }

    iter->index++;

    return fr.size;
}

ssize_t av_fifo_read(AVFifo *fifo, void *data, size_t size, AVFrameIter *iter)
{
    struct iovec iov = {
        .iov_base = data,
        .iov_len  = size,
    };

    return av_fifo_readv(fifo, &iov, 1, iter);
}

static int av_fifo_drop(AVFifo *fifo)
{
    AVFrame fr;
    av_fifo_read_internal(fifo, (uint8_t *)&fr, fifo->rptr, sizeof(AVFrame));

    fifo->rptr = seek_addr(fifo, fifo->rptr, sizeof(AVFrame) + fr.size);
    fifo->rndx += sizeof(AVFrame) + fr.size;
    fifo->first++;

    return 0;
}

static ssize_t av_fifo_write_internal(AVFifo *fifo, uint8_t *dst, uint8_t *src, size_t size)
{
    int left = size;

    do {
        int len = min(fifo->bufend - dst, left);
        memcpy(dst, src, len);
        src += len;
        dst += len;
        if (dst >= fifo->bufend)
            dst = fifo->buffer;
        left -= len;
    } while (left > 0);

    return size;
}

ssize_t av_fifo_writev(AVFifo *fifo, struct iovec *iov, int iovcnt)
{
    ssize_t len;

    int datsiz = iovec_size(iov, iovcnt);
    if (datsiz < 0)
        return -1;

    int needsize = sizeof(AVFrame) + datsiz;
    if (needsize > fifo->bufsiz)
        return -1;

    int n;
    while (needsize > (n = av_fifo_space(fifo))) {
        /* haven't got enought space, drop eldest one */
        av_fifo_drop(fifo);
    }

    /* write frame head */
    AVFrame fr = {
        .size  = datsiz
    };

    len = av_fifo_write_internal(fifo, fifo->wptr, (uint8_t *)&fr, sizeof(AVFrame));
    fifo->wptr = seek_addr(fifo, fifo->wptr, len);
    fifo->wndx += len;

    int i;
    for (i = 0; i < iovcnt; i++) {
        /* write frame body */
        len = av_fifo_write_internal(fifo, fifo->wptr, iov[i].iov_base, iov[i].iov_len);
        fifo->wptr = seek_addr(fifo, fifo->wptr, len);
        fifo->wndx += len;
    }

    fifo->next++;

    return datsiz;
}

ssize_t av_fifo_write(AVFifo *fifo, void *data, size_t size)
{
    struct iovec iov = {
        .iov_base = data,
        .iov_len  = size,
    };

    return av_fifo_writev(fifo, &iov, 1);
}

int av_fifo_erase(AVFifo *fifo)
{
    while (fifo->first != fifo->next)
        av_fifo_drop(fifo);
    return 0;
}

int av_fifo_length(AVFifo *fifo)
{
    return fifo->next - fifo->first;
}

int av_fifo_last(AVFifo *fifo, AVFrameIter *iter)
{
    /* invalid frame index, point it to the first frame */
    iter->index = fifo->next;
    iter->frame = fifo->wptr;
    return 0;
}

ssize_t av_fifo_write_generic(AVFifo *fifo, size_t maxsize, ssize_t (*write_cb)(struct iovec *iov, int iovcnt, void *ctx), void *ctx)
{
    if (!fifo || !write_cb)
        return -1;

    int needsize = sizeof(AVFrame) + maxsize;
    if (needsize > fifo->bufsiz)
        return -1;

    int n;
    while (needsize > (n = av_fifo_space(fifo))) {
        /* haven't got enought space, drop eldest one */
        av_fifo_drop(fifo);
    }

    AVFrame *fptr = (AVFrame *)fifo->wptr;

    struct iovec iov[2];
    int iovcnt = 1;

    iov[0].iov_base = seek_addr(fifo, (uint8_t *)fptr, sizeof(AVFrame));

    int left = fifo->bufend - (uint8_t *)iov[0].iov_base;

    if (left < maxsize) {
        iov[0].iov_len  = left;
        iov[1].iov_base = seek_addr(fifo, iov[0].iov_base, iov[0].iov_len);
        iov[1].iov_len  = maxsize - iov[0].iov_len;
        iovcnt++;
    } else {
        iov[0].iov_len  = maxsize;
    }

    int datsiz = write_cb(iov, iovcnt, ctx);
    if (datsiz <= 0)
        return datsiz;

    /* write frame head */
    AVFrame f = { .size  = datsiz };

    ssize_t len = av_fifo_write_internal(fifo, (uint8_t *)fptr, (uint8_t *)&f, sizeof(AVFrame));
    fifo->wptr = seek_addr(fifo, fifo->wptr, len);
    fifo->wndx += len;

    /* write frame body */
    fifo->wptr = seek_addr(fifo, fifo->wptr, datsiz);
    fifo->wndx += datsiz;

    fifo->next++;

    return datsiz;
}

int av_fifo_foreach(AVFifo *fifo, AVFrameIter *iter, int (*cb)(struct iovec *iov, int iovcnt, void *priv_data), void *priv_data)
{
    AVFrameIter cursor = *iter;

    while (1) {
        if (av_fifo_left(fifo, &cursor) == 0)
            break;

        /* read frame head */
        AVFrame fr;
        av_fifo_read_internal(fifo, (uint8_t *)&fr, cursor.frame, sizeof(AVFrame));

        cursor.frame = seek_addr(fifo, cursor.frame, sizeof(AVFrame));

        struct iovec iov[2];

        int i, left, size;
        for (i = 0, left = fr.size; i < 2 && left > 0; i++, left -= size) {
            size = min(fifo->bufend - (uint8_t *)cursor.frame, left);

            iov[i].iov_base = cursor.frame;
            iov[i].iov_len  = size;

            cursor.frame = seek_addr(fifo, cursor.frame, size);
        }

        int ret = cb(iov, i, priv_data);
        if (ret < 0)
            break;
        else if (ret > 0)
            break;

        cursor.index++;
    }

    return 0;
}

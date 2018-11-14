#ifndef __FIFO_H__
#define __FIFO_H__

typedef struct AVFifo AVFifo;

typedef struct AVFrameIter {
    int   cached;
    int   index;
    void *frame;
} AVFrameIter;

AVFifo *av_fifo_create(int size);
int av_fifo_destroy(AVFifo *fifo);

ssize_t av_fifo_write(AVFifo *fifo, void *data, size_t size);
ssize_t av_fifo_read (AVFifo *fifo, void *data, size_t size, AVFrameIter *iter);

ssize_t av_fifo_writev(AVFifo *fifo, struct iovec *iov, int iovcnt);
ssize_t av_fifo_readv (AVFifo *fifo, struct iovec *iov, int iovcnt, AVFrameIter *iter);

int av_fifo_erase(AVFifo *fifo);
int av_fifo_length(AVFifo *fifo);

int av_fifo_left(AVFifo *fifo, AVFrameIter *iter);
int av_fifo_last(AVFifo *fifo, AVFrameIter *iter);

AVFrameIter av_fifo_next_frame(AVFifo *fifo, int cached);
AVFrameIter av_fifo_first_frame(AVFifo *fifo, int cached);

ssize_t av_fifo_write_generic(AVFifo *fifo, size_t maxsize, ssize_t (*write_cb)(struct iovec *iovs, int iovcnt, void *ctx), void *ctx);

int av_fifo_foreach(AVFifo *fifo, AVFrameIter *iter, int (*cb)(struct iovec *iov, int iovcnt, void *priv_data), void *priv_data);

#endif /* __FIFO_H__ */

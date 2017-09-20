/******************************************************************
 * File Name    : bufchan.h
 * Description  : audio/video buffer pool implementation
 * Author       : <huangchengman@detu.com>
 * Date         : 2016-07-04
 ******************************************************************/

#ifndef __BUFCHAN_H_
#define __BUFCHAN_H_

#include <stdint.h>
#include <sys/uio.h>

#define AV_CACHED         (0x1)
#define AV_RDONLY         (0x2)
#define AV_WRONLY         (0x4)

int BufChan_MkFifo(char *name, size_t size);
int BufChan_UnLink(char *name);

long BufChan_Open(char *name, int mode);
int BufChan_Close(long handle);

ssize_t BufChan_Write(long handle, void *buf, size_t nbyte);
ssize_t BufChan_Read (long handle, void *buf, size_t nbyte, int milliseconds);

ssize_t BufChan_WriteV(long handle, struct iovec *iov, size_t iovcnt);
ssize_t BufChan_ReadV (long handle, struct iovec *iov, size_t iovcnt, int milliseconds);

ssize_t BufChan_WriteBlock(long handle, size_t maxsize, ssize_t (*write_cb)(struct iovec *iovs, int iovcnt, void *priv_data), void *priv_data);

int BufChan_Left(long handle);
int BufChan_ForEach(long handle, int (*cb)(struct iovec *iovs, int iovcnt, void *priv_data), void *priv_data);

#endif /* __BUFCHAN_H_ */

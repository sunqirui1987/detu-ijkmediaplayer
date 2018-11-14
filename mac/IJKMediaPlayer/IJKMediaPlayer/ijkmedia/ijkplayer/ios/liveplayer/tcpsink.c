/******************************************************************
 * File Name    : tcpsink.c
 * Description  : video transfor by tcp
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-08
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>

#include "frame.h"
#include "netio.h"
#include "viddec.h"
#include "tcpsink.h"
#include "bufchan.h"

#define min(x, y)    ((x) < (y) ? (x) : (y))

struct TcpSink {
    char stream_name[64];
    char ipaddr[512];
    int r_handle;
    int port;
    uint8_t buffer[1024 * 1024];

    pthread_t sink_tid;
    int abort;
};

static int sinker(TcpSink *sink)
{
    printf("ipaddr:%s, port:%d \n",sink->ipaddr, sink->port);
    int sockfd = j_net_tcp_connect("192.168.3.60", 8554);
    if (sockfd < 0) {
        printf("tcp connect [%s] failed\n",sink->ipaddr);
        return -1;
    }
    printf("sinker tcp connect succ\n");
    long handle = BufChan_Open(sink->stream_name, AV_WRONLY);
    if (!handle) {
        close(sockfd);
        printf("open bufchan failed\n");
	return -1;
    }

    printf("sinker tcp connect succ 2\n");
    int bufsiz = 32 * 1024;
    int len;

    uint8_t *buffer = malloc(bufsiz);
    if (!buffer) {
        close(sockfd);
        BufChan_Close(handle);
        return -1;
    }

    MediaFrame f;
    printf("sinker tcp connect succ 3\n");
    while (!sink->abort) {
        printf("sinker tcp read bitdata 1\n");
        if (j_net_readn_timedwait(sockfd, &f, sizeof(MediaFrame)) < 0)
            break;
        printf("sinker tcp read bitdata 2\n");
        if (!buffer || bufsiz < f.rawsize) {
            if (buffer)
                free(buffer);
            buffer = malloc(f.rawsize);
            bufsiz = f.rawsize;
        }
        printf("sinker tcp read bitdata 3\n");
        if (j_net_readn_timedwait(sockfd, buffer, f.rawsize) != f.rawsize)
            break;

        struct iovec iov;
        //iov[0].iov_base = &f;
        //iov[0].iov_len  = sizeof(MediaFrame);
        iov.iov_base = buffer;
        iov.iov_len  = f.rawsize;
	
        printf("==>to bitstream buf,size:%d \n",f.rawsize);
        BufChan_WriteV(handle, &iov, 1);
    }

    BufChan_Close(handle);
    close(sockfd);
    free(buffer);

    return 0;
}

static void *sink_service(void *arg)
{
    TcpSink *sink = (TcpSink *)arg;
    if (!sink)
        return NULL;

    if (!sink->abort) {
        sinker(sink);
        sleep(1);
    }

    return NULL;
}

TcpSink *TcpSink_OpenStream(char *ipaddr, int port)
{
    TcpSink *sink = (TcpSink *)malloc(sizeof(TcpSink));
    if (!sink)
        return NULL;
    memset(sink, 0, sizeof(TcpSink));
    //sink->ipaddr = ipaddr;
    strcpy(sink->ipaddr, ipaddr);
    sink->port   = port;
    snprintf(sink->stream_name, sizeof(sink->stream_name), "tcpsink.video.%p", sink);
    BufChan_MkFifo(sink->stream_name, 512 * 1024);

    sink->r_handle = BufChan_Open(sink->stream_name, AV_RDONLY);
    if (!sink->r_handle)
        goto fail;

    pthread_create(&sink->sink_tid, NULL, sink_service, sink);

    return sink;

fail:
    BufChan_Close(sink->r_handle);
    BufChan_UnLink(sink->stream_name);
    free(sink);
    return NULL;
}

int TcpSink_CloseStream(TcpSink *sink)
{
    sink->abort = 1;
    pthread_join(sink->sink_tid, NULL);

    BufChan_Close(sink->r_handle);
    BufChan_UnLink(sink->stream_name);
    free(sink);

    return 0;
}

int TcpSink_ReadStream(TcpSink *sink, VideoPkt *pkt, int time_ms)
{
    if (!sink->r_handle || !sink->buffer) {
    	printf("sink r_handle is NULL or sink buffer is NULL!\n");
    }
    long handle = BufChan_Open(sink->stream_name, AV_RDONLY);
    int len = BufChan_Read(handle, sink->buffer, sizeof(sink->buffer), time_ms);
    if (len <= 0)
        return len;

    MediaFrame *mf = (MediaFrame *)sink->buffer;

    memset(pkt, 0, sizeof(VideoFrame));
    pkt->pts      = mf->pts;
    pkt->keyframe = mf->keyframe;
    pkt->data     = sink->buffer;//mf->rawdata;//sink->buffer + sizeof(VideoFrame)
    pkt->size     = len;//mf->rawsize;//len - sizeof(VideoFrame)
    
    printf("==>to decode,pkt size:%d \n",pkt->size);
    return 1;
}

static int check_keyframe(struct iovec *iov, int iovcnt, void *priv_data)
{
    if (iovcnt == 1) {
        MediaFrame *f = (MediaFrame *)iov[0].iov_base;
        if (f->keyframe) {
            *(int *)priv_data = 1;
            return 1;
        }
    } else if (iovcnt > 1) {
        MediaFrame f;
        uint8_t *ptr = (uint8_t *)&f;

        int i, left, size;
        for (i = 0, left = sizeof(MediaFrame); i < iovcnt && left > 0; i++, left -= size) {
            size = min(left, iov[i].iov_len);
            memcpy(ptr, iov[i].iov_base, size);
        }

        if (f.keyframe) {
            *(int *)priv_data = 1;
            return 1;
        }
    }

    return 0;
}

int TcpSink_HasKeyFrameBuffered(TcpSink *sink)
{
    int got_keyframe = 0;
    BufChan_ForEach(sink->r_handle, check_keyframe, &got_keyframe);
    return got_keyframe;
}

int TcpSink_NumberFrameBuffered(TcpSink *sink)
{
    return BufChan_Left(sink->r_handle);
}

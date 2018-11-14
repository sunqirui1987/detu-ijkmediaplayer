/******************************************************************
 * File Name    : videosink.c
 * Description  : video sink
 * Author       : huangchengman <huangchengman@detu.com>
 * Date         : 2016-07-07
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "url.h"
#include "rtspdec.h"
#include "tcpsink.h"
#include "videosink.h"

#define NELEMS(x)       (sizeof(x) / sizeof((x)[0]))

typedef struct SinkCore {
    char *Name;

    void (*Init)(void);
    int (*Quit)(void);

    void *(*OpenStream)(char *url);
    int  (*CloseStream)(void *handle);

    int (*ReadStream)(void *handle, VideoPkt *pkt, int time_ms);

    int (*HasKeyFrameBuffered)(void *handle);
    int (*NumberFrameBuffered)(void *handle);
} SinkCore;

struct VideoSink {
    UrlData *url;
    SinkCore *core;

    void *handle;
};

static int rtspsink_readstream(void *handle, VideoPkt *pkt, int time_ms)
{
    RtspFrame rf;
    int ret = RtspDec_ReadStream((RtspDec *)handle, &rf, time_ms);
    if (ret <= 0)
        return ret;

    memset(pkt, 0, sizeof(VideoFrame));
    pkt->pts      = rf.pts;
    pkt->keyframe = rf.keyframe;
    pkt->data     = rf.data;
    pkt->size     = rf.size;
    pkt->pkt_num = rf.pkt_num;
    return 1;
}

static SinkCore RtspSinkCore = {
    .Name = "rtsp",

    .Init = RtspDec_Init,
    .Quit = RtspDec_Quit,

    .OpenStream  = RtspDec_OpenStream,
    .CloseStream = RtspDec_CloseStream,
    .ReadStream  = rtspsink_readstream,

    .HasKeyFrameBuffered = RtspDec_HasKeyFrameBuffered,
    .NumberFrameBuffered = RtspDec_NumberFrameBuffered,
};

static SinkCore TcpSinkCore = {
    .Name = "tcp",

    .OpenStream  = TcpSink_OpenStream,
    .CloseStream = TcpSink_CloseStream,
    .ReadStream  = TcpSink_ReadStream,

    .HasKeyFrameBuffered = TcpSink_HasKeyFrameBuffered,
    .NumberFrameBuffered = TcpSink_NumberFrameBuffered,
};

static SinkCore *AllSinkCores[] = {
    &RtspSinkCore,
    &TcpSinkCore,
};

VideoSink *VideoSink_OpenStream(char *url)
{
    VideoSink *sink = (VideoSink *)malloc(sizeof(VideoSink));
    if (!sink)
        return NULL;
    memset(sink, 0, sizeof(VideoSink));
   
    sink->url = UrlParse(url);
    if (!sink->url) {
        free(sink);
        return NULL;
    }

    int i;
    for (i = 0; i < NELEMS(AllSinkCores); i++) {
        SinkCore *core = AllSinkCores[i];
        if (strcmp(sink->url->scheme, core->Name) == 0) {
            sink->core = core;
            break;
        }
    }
    sink->core = AllSinkCores[0];
    if (!sink->core) {
        UrlFree(sink->url);
        free(sink);
        return NULL;
    }
    
    sink->handle = sink->core->OpenStream(url);
    if (!sink->handle) {
        UrlFree(sink->url);
        free(sink);
        printf("open tcp link failed\n");
        return NULL;
    }

    return sink;
}

int VideoSink_CloseStream(VideoSink *sink)
{
    printf("VideoSink_CloseStream()##########1\n");
    if (sink->handle)
        sink->core->CloseStream(sink->handle);
    
    printf("VideoSink_CloseStream()##########2\n");
    UrlFree(sink->url);
    free(sink);
    printf("VideoSink_CloseStream()##########3\n");
    return 0;
}

int VideoSink_ReadStream(VideoSink *sink, VideoPkt *pkt, int time_ms)
{
    return sink->core->ReadStream(sink->handle, pkt, time_ms);
}

int VideoSink_HasKeyFrameBuffered(VideoSink *sink)
{
    return sink->core->HasKeyFrameBuffered(sink->handle);
}

int VideoSink_NumberFrameBuffered(VideoSink *sink)
{
    return sink->core->NumberFrameBuffered(sink->handle);
}

int VideoSink_Init(void)
{
    int i;
    for (i = 0; i < NELEMS(AllSinkCores); i++) {
        SinkCore *core = AllSinkCores[i];
        if (core->Init)
            core->Init();
    }

    return 0;
}

int VideoSink_Quit(void)
{
    int i;
    for (i = 0; i < NELEMS(AllSinkCores); i++) {
        SinkCore *core = AllSinkCores[i];
        if (core->Quit)
            core->Quit();
    }

    return 0;
}

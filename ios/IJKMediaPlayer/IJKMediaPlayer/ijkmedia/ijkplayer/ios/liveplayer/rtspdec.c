/******************************************************************
 * File Name    : rtspdec.c
 * Description  : rtsp demuxer
 * Author       : huangchengman@detu.com
 * Date         : 2016-04-12
 ******************************************************************/
//#define ANDROID_APP
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>

#include "rtspdec.h"
#include "bufchan.h"

#ifdef ANDROID_APP
#define TAG "RTSPDec"
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  TAG, format, ##__VA_ARGS__)
#endif

#define MAX_FRAME_SIZE  (512 * 1024)

#define min(x, y)   ((x) < (y) ? (x) : (y))

#define MAX_READ_TIMEOUT (3600)
typedef struct {
    int64_t pts;
    int keyframe;
    int stream_index;
    int frame_cnt;
    int refer;
} RawFrame;

typedef struct RtspSession {
    char url[1024];
    char bufname[64];
    int refcnt;

    pthread_t stream_tid;
    int abort;

    struct RtspSession *next;
} RtspSession;

static RtspSession *first_session = NULL;
static pthread_mutex_t session_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t read_lock;
static pthread_cond_t read_cond;

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


static int interrupt_cb(void *ctx)
{
    struct timespec tp;
    int ret;

    ret = time_after(&tp, MAX_READ_TIMEOUT);//set 1s is timeout
    pthread_mutex_lock(&read_lock);
    ret = pthread_cond_timedwait(&read_cond, &read_lock, &tp);
    if (ret == ETIMEDOUT) {
	pthread_mutex_unlock(&read_lock);
	return -1;
    }
    else {
	pthread_mutex_unlock(&read_lock);
    	return 0;
    }
}

static const AVIOInterruptCB int_cb = { interrupt_cb, NULL};

static int rtsp_service(RtspSession *s)
{
    printf("rtsp service start\n");
    long bufhndl = 0;
    int video_stream_idx = -1;
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *dec_ctx = NULL;
    
    int ret;
    
    //force rtsp by tcp transport
//    AVDictionary *option = NULL;
//    av_dict_set(&option, "rtsp_transport", "tcp", 0);

//    pthread_mutex_init(&read_lock, NULL);
//    pthread_cond_init(&read_cond, NULL);
//	
//    fmt_ctx->flags = AVFMT_FLAG_NONBLOCK;
//    fmt_ctx = avformat_alloc_context();
//    fmt_ctx->interrupt_callback = int_cb;
//    fmt_ctx->flags |= AVFMT_FLAG_NONBLOCK;

//    if (avformat_open_input(&fmt_ctx, s->url, NULL, &option) < 0)
    if (avformat_open_input(&fmt_ctx, s->url, NULL, NULL) < 0)
        goto fail;

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
        goto fail;

    video_stream_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_idx < 0)
        goto fail;

    dec_ctx = fmt_ctx->streams[video_stream_idx]->codec;
    bufhndl = BufChan_Open(s->bufname, AV_WRONLY);
    if (!bufhndl)
        goto fail;

    int pkt_cnt = 0;
    while (!s->abort) {
        AVPacket pkt = {0};
        av_init_packet(&pkt);
        //add interrupt cb,sometime av_read_frame() will blocking always
        //so add read timeout
        ret = av_read_frame(fmt_ctx, &pkt);
        if (ret < 0) {
            printf("av_read_frame failed!!!,ret:%d",ret);
            break;
        }

        if (pkt.stream_index != video_stream_idx) {  /* video only */
            av_free_packet(&pkt);
            continue;
        }
        
        RawFrame f = {
            .pts          = pkt.pts,
            .keyframe     = !!(pkt.flags & AV_PKT_FLAG_KEY),
            .stream_index = pkt.stream_index,
            .frame_cnt = pkt_cnt++,
        };
       
        struct iovec iov[2];
        iov[0].iov_base = &f;
        iov[0].iov_len  = sizeof(RawFrame);
        iov[1].iov_base = pkt.data;
        iov[1].iov_len  = pkt.size;
        //printf("rtsp service recv pkt,pkt size:%d,pkt_no:%d,pkt data ptr:%p \n",pkt.size,pkt_cnt,pkt.data);
        ret = BufChan_WriteV(bufhndl, iov, 2);
        av_free_packet(&pkt);
    }

fail:
    pthread_cond_destroy(&read_cond);
    pthread_mutex_destroy(&read_lock);

    if (fmt_ctx)
        avformat_close_input(&fmt_ctx);
    if (bufhndl)
        BufChan_Close(bufhndl);

    return 0;
}

static void *video_stream_thread(void *arg)
{
    RtspSession *s = (RtspSession *)arg;

    while (!s->abort) {
        rtsp_service(s);
        usleep(10);
    }

    return NULL;
}

static RtspSession *rtsp_find_session(char *url)
{
    RtspSession *s;

    for (s = first_session; s; s = s->next) {
        if (strncmp(url, s->url, sizeof(s->url)) == 0)
            return s;
    }

    return NULL;
}

static RtspSession *rtsp_create_session(char *url)
{
    pthread_mutex_lock(&session_lock);

    RtspSession *s = rtsp_find_session(url);
    if (s) {
        s->refcnt++;    /* increase referenct count */
        pthread_mutex_unlock(&session_lock);
        return s;
    }

    s = (RtspSession *)malloc(sizeof(RtspSession));
    if (!s) {
        pthread_mutex_unlock(&session_lock);
        return NULL;
    }
    memset(s, 0, sizeof(RtspSession));

    strncpy(s->url, url, sizeof(s->url) - 1);
    snprintf(s->bufname, sizeof(s->bufname), "bufchan-%p", s);

    BufChan_MkFifo(s->bufname, MAX_FRAME_SIZE);
    //LOGI("rtsp stream buf size:%d\n", MAX_FRAME_SIZE);
    s->refcnt = 1;

    pthread_create(&s->stream_tid, NULL, video_stream_thread, s);

    s->next = first_session;
    first_session = s;

    pthread_mutex_unlock(&session_lock);

    return s;
}

static int rtsp_destroy_session(RtspSession *s)
{
    pthread_mutex_lock(&session_lock);

    s->refcnt--;
    if (s->refcnt > 0) {
        pthread_mutex_unlock(&session_lock);
        return 0;
    }

    RtspSession *s1, **ss;

    /* remove rtsp session from list */
    ss = &first_session;
    while ((*ss) != NULL) {
        s1 = *ss;
        if (s1 != s)
            ss = &s1->next;
        else {
            *ss = s->next;
            break;
        }
    }

    pthread_mutex_unlock(&session_lock);

    s->abort = 1;
    pthread_join(s->stream_tid, NULL);

    BufChan_UnLink(s->bufname);
    free(s);

    return 0;
}

struct RtspDec {
    char url[1024];

    RtspSession *session;

    long bufhndl;

    int64_t pts_base;
    int64_t timebase;

    uint8_t buffer[1024 * 1024];
};

static int64_t get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
}

int RtspDec_Init(void)
{
    av_register_all();
    avformat_network_init();

    return 0;
}

int RtspDec_Quit(void)
{
    return 0;
}

RtspDec *RtspDec_OpenStream(char *url)
{
    RtspDec *dec = (RtspDec *)malloc(sizeof(RtspDec));
    if (!dec)
        return NULL;
    memset(dec, 0, sizeof(RtspDec));

    dec->session = rtsp_create_session(url);
    if (!dec->session)
        goto fail;

    dec->pts_base = -1;
    dec->timebase = -1;

    dec->bufhndl = BufChan_Open(dec->session->bufname, AV_RDONLY);
    if (!dec->bufhndl)
        goto fail;

    return dec;

fail:
    RtspDec_CloseStream(dec);
    return NULL;
}

int RtspDec_CloseStream(RtspDec *dec)
{
    printf("RtspDec_CloseStream#####\n");
    if (dec->bufhndl){
        BufChan_Close(dec->bufhndl);
    }
    if (dec->session){
        rtsp_destroy_session(dec->session);
    }
    
    free(dec);

    return 0;
}

static int64_t needwait(RtspDec *ctx, int64_t pts)
{
    if (ctx->pts_base == -1 || ctx->pts_base > pts) {
        ctx->pts_base = pts;
        ctx->timebase = get_time();
    }

    double itv_pts = pts - ctx->pts_base;
    int64_t next_t = itv_pts / 90000 * 1000 + ctx->timebase;

    int64_t time_ms = next_t - get_time();
    if (time_ms < 50 && time_ms >= 0)
        return time_ms;

    ctx->pts_base = pts;
    ctx->timebase = get_time();

    return 0;
}

int RtspDec_ReadStream(RtspDec *dec, RtspFrame *frame, int time_ms)
{
    int len = BufChan_Read(dec->bufhndl, dec->buffer, sizeof(dec->buffer), time_ms);
    if (len <= 0) {
        return len;
    }

    RawFrame *rf = (RawFrame *)dec->buffer;

    memset(frame, 0, sizeof(RtspFrame));
    frame->codec    = 0;
    frame->pts      = rf->pts;
    frame->keyframe = rf->keyframe;
    frame->data     = dec->buffer + sizeof(RawFrame);
    frame->size     = len - sizeof(RawFrame);
    frame->pkt_num = rf->frame_cnt;
    //printf("RtspDec_ReadStream, get pkt size:%d,pkt num:%d",frame->size, frame->pkt_num);

//    int wait_time = needwait(dec, rf->pts);
//    if (wait_time > 0) {
//        //printf("read frame delay time:%d(ms)", wait_time);
//        usleep(wait_time);
//    }
    return 1;
}

static int check_keyframe(struct iovec *iov, int iovcnt, void *priv_data)
{
    if (iovcnt == 1) {
        RawFrame *f = (RawFrame *)iov[0].iov_base;
        if (f->keyframe) {
            *(int *)priv_data = 1;
            return 1;
        }
    } else if (iovcnt > 1) {
        RawFrame f;
        uint8_t *ptr = (uint8_t *)&f;

        int i, left, size;
        for (i = 0, left = sizeof(RawFrame); i < iovcnt && left > 0; i++, left -= size) {
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

int RtspDec_HasKeyFrameBuffered(RtspDec *dec)
{
    int got_keyframe = 0;
    BufChan_ForEach(dec->bufhndl, check_keyframe, &got_keyframe);
    return got_keyframe;
}

int RtspDec_NumberFrameBuffered(RtspDec *dec)
{
    return BufChan_Left(dec->bufhndl);
}

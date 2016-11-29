//
// Created by chao on 2016/11/29.
//

#include "ffmpegutil.h"
#include <stdio.h>
#include <stdlib.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavfilter/avfilter.h"
#include "libavutil/imgutils.h"
#include "log.h"

bool videoGetThumb(const char *fileAbsolutePath, void **outData, int *outSize, int *outWidth,
                   int *outHeight) {
    if (strlen(fileAbsolutePath) == 0) {
        return false;
    }
    av_register_all();
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (avformat_open_input(&pFormatContext, fileAbsolutePath, NULL, NULL) != 0) {
        return false;
    }
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        return false;
    }
    int posVideoStream = -1;
    const int NUMBER_STREAM = pFormatContext->nb_streams;
    for (int i = 0; i < NUMBER_STREAM; i++) {
        if (AVMEDIA_TYPE_VIDEO
            == pFormatContext->streams[i]->codec->codec_type) {
            posVideoStream = i;
        }
    }
    if (posVideoStream == -1) {
        return false;
    }
    AVStream *pStream = pFormatContext->streams[posVideoStream];
    AVCodecContext *pCodecContext = pStream->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if (pCodec == NULL) {
        return false;
    }
    if (0 != avcodec_open2(pCodecContext, pCodec, NULL)) {
        return false;
    }
    const int picWidth = pCodecContext->width;
    const int picHeight = pCodecContext->height;

    *outWidth = picWidth;
    *outHeight = picHeight;

    const int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, picWidth, picHeight, 1);
    *outSize = bufferSize;
    *outData = (void *) malloc(bufferSize);
    uint8_t *outBuffer = (uint8_t *) av_malloc(bufferSize);
    AVFrame *pFrameYuv = av_frame_alloc();
    av_image_fill_arrays(pFrameYuv->data, pFrameYuv->linesize, outBuffer,
                         AV_PIX_FMT_RGBA, picWidth, picHeight, 1);
    AVPacket *pPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *pFrame = av_frame_alloc();
    struct SwsContext *pSwsContext = sws_getContext(picWidth, picHeight, pCodecContext->pix_fmt,
                                                    picWidth, picHeight, AV_PIX_FMT_RGBA,
                                                    SWS_BICUBIC, NULL, NULL, NULL);
    int gotPicture = -1;
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        if (pPacket->stream_index != posVideoStream) {
            continue;
        }
        if (avcodec_decode_video2(pCodecContext, pFrame, &gotPicture, pPacket) < 0) {
            continue;
        }
        if (gotPicture <= 0) {
            continue;
        }
        sws_scale(pSwsContext, (const uint8_t *const *) pFrame->data,
                  pFrame->linesize, 0, pCodecContext->height, pFrameYuv->data,
                  pFrameYuv->linesize);
        if (pFrameYuv->key_frame) {
            memcpy(*outData, pFrameYuv->data[0], bufferSize);
            goto finish;
        }
    }
    finish:
    sws_freeContext(pSwsContext);
    av_free(pPacket);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameYuv);
    avcodec_close(pCodecContext);
    avformat_close_input(&pFormatContext);
    return true;
}
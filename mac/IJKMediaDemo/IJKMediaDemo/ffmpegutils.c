//
// Created by chao on 2017/3/3.
//

#include "ffmpegutils.h"
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>

bool ffmpegUtilsSws(int srcPixFmt, unsigned char *srcData, int srcWidth, int srcHeight,
                    int dstPixFmt, unsigned char *dstData, int dstWidth, int dstHeight) {
    struct SwsContext *sws_ctx = sws_getContext(
            srcWidth,
            srcHeight,
            srcPixFmt,
            dstWidth,
            dstHeight,
            dstPixFmt,
            SWS_BICUBIC,
            NULL, NULL, NULL);
    if (sws_ctx == NULL) {
        return false;
    }
    AVFrame *avFrameSrc = av_frame_alloc();
    av_image_fill_arrays(avFrameSrc->data, avFrameSrc->linesize, srcData,
                         srcPixFmt, srcWidth, srcHeight, 1);
    AVFrame *avFrameDst = av_frame_alloc();
    av_image_fill_arrays(avFrameDst->data, avFrameDst->linesize, dstData,
                         dstPixFmt, dstWidth, dstHeight, 1);
    sws_scale(sws_ctx, avFrameSrc->data,
              avFrameSrc->linesize,
              0, srcHeight, avFrameDst->data,
              avFrameDst->linesize);
    sws_freeContext(sws_ctx);
    av_frame_free(&avFrameSrc);
    av_frame_free(&avFrameDst);
    return true;
}

bool ffmpegUtilsYuv420pToRgba(unsigned char *yuv420pData, int yuvWidth, int yuvHeight,
                              unsigned char *rgbaData, int rgbaWidth, int rgbaHeight) {
    return ffmpegUtilsSws(AV_PIX_FMT_YUV420P, yuv420pData, yuvWidth, yuvHeight, AV_PIX_FMT_RGB24,
                          rgbaData, rgbaWidth, rgbaHeight);
}

bool ffmpegUtilsRgbaToYuv420p(unsigned char *rgbaData, int rgbaWidth, int rgbaHeight,
                              unsigned char *yuv420pData, int yuvWidth, int yuvHeight) {
    return ffmpegUtilsSws(AV_PIX_FMT_RGB32, rgbaData, rgbaWidth, rgbaHeight, AV_PIX_FMT_YUV420P,
                          yuv420pData, yuvWidth, yuvHeight);
}
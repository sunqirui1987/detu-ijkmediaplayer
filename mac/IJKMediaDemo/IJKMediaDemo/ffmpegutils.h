//
// Created by chao on 2017/3/3.
//

#ifndef ANDROID_FFMPEGUTILS_H
#define ANDROID_FFMPEGUTILS_H

#include <stdbool.h>
#include <libavutil/pixfmt.h>

bool ffmpegUtilsYuv420pToRgba(unsigned char *yuv420pData, int yuvWidth, int yuvHeight, unsigned char *rgbaData, int rgbaWidth, int rgbAHeight);

bool ffmpegUtilsRgbaToYuv420p(unsigned char *rgbaData, int rgbaWidth, int rgbAHeight, unsigned char *yuv420pData, int yuvWidth, int yuvHeight);

bool ffmpegUtilsSws(int srcPixFmt, unsigned char* srcData, int srcWidth, int srcHeight, int sdtPixFmt, unsigned char* dstData, int dstWidth, int dstHeight);

#endif //ANDROID_FFMPEGUTILS_H

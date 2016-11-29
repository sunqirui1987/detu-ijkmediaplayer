//
// Created by chao on 2016/11/29.
//

#ifndef IJKPLAYER_FFMPEGUTIL_H
#define IJKPLAYER_FFMPEGUTIL_H

#include <stdbool.h>

/**
 * 获取视频一帧缩略图，输出rgb数据
 */
bool videoGetThumb(const char* fileAbsolutePath, void** outData, int* outSize, int* outWidth, int* outHeight);

#endif //IJKPLAYER_FFMPEGUTIL_H

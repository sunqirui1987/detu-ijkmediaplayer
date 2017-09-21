//
//  ijk_ffplay_decoder.c
//  IJKMediaPlayer
//
//  Created by chao on 2017/6/12.
//  Copyright © 2017年 detu. All rights reserved.
//

#include "ijk_ffplay_decoder.h"
#include <stdlib.h>
#include <string.h>
#import <CoreVideo/CVPixelBuffer.h>

struct IjkFfplayDecoder {
    void* opaque;
    char* fileAbsolutePath;
};

int ijkFfplayDecoder_init(void) {
    return 0;
}

int ijkFfplayDecoder_uninit(void) {
    return 0;
}

IjkFfplayDecoder *ijkFfplayDecoder_create(void) {
    IjkFfplayDecoder* decoder = (IjkFfplayDecoder*)calloc(1, sizeof(IjkFfplayDecoder));
    return decoder;
}

int ijkFfplayDecoder_setLogLevel(IJKLogLevel log_level) {
    return 0;
}

int ijkFfplayDecoder_setLogCallback(void(*callback)(void*, int, const char*, va_list)) {
    return 0;
}

int ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, void* opaque, IjkFfplayDecoderCallBack* callback) {
    return 0;
}

int ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* file_absolute_path) {
    return 0;
}

int ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_start(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec) {
    return 0;
}

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder) {
    return false;
}

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder) {
    return 0l;
}

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder) {
    return 0l;
}

int ijkFfplayDecoder_release(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float volume) {
    return 0;
}

float ijkFfplayDecoder_getVolume(IjkFfplayDecoder* decoder) {
    return 0;
}

int ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, long value) {
    return 0;
}

int ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, const char* value) {
    return 0;
}

int ijkFfplayDecoder_getVideoCodecInfo(IjkFfplayDecoder* decoder, char **codec_info) {
    return 0;
}

int ijkFfplayDecoder_getAudioCodecInfo(IjkFfplayDecoder* decoder, char **codec_info) {
    return 0;
}

long ijkFfplayDecoder_getPropertyLong(IjkFfplayDecoder* decoder, int id, long default_value) {
    return 0l;
}

float ijkFfplayDecoder_getPropertyFloat(IjkFfplayDecoder* decoder, int id, float default_value) {
    return 0.0;
}

int ijkFfplayDecoder_getMediaMeta(IjkFfplayDecoder* decoder, IjkMetadata* metadata) {
    return 0;
}

//decoder_name: h264_vtb
int ijkFfplayDecoder_setHwDecoderName(IjkFfplayDecoder* decoder, const char* decoder_name) {
    return 0;
}

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
#import "IJKFFMoviePlayerController.h"
#import <CoreVideo/CVPixelBuffer.h>
#import <AppKit/AppKit.h>
#include "ijksdl.h"
#include "ijksdl_vout_overlay_videotoolbox.h"

struct IjkFfplayDecoder {
    void* opaque;
    char* fileAbsolutePath;
    IjkFfplayDecoderCallBack* callBack;
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

int ijkFfplayDecoder_setLogLevel(IJKLogLevel log_level);

int ijkFfplayDecoder_setLogCallback(void(*callback)(void*, int, const char*, va_list));

int ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, void* opaque, IjkFfplayDecoderCallBack* callback);

int ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* file_absolute_path);

int ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_start(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec);

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder);

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder);

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_release(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float volume);

float ijkFfplayDecoder_getVolume(IjkFfplayDecoder* decoder);

int ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, long value);

int ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, const char* value);

int ijkFfplayDecoder_getVideoCodecInfo(IjkFfplayDecoder* decoder, char **codec_info);

int ijkFfplayDecoder_getAudioCodecInfo(IjkFfplayDecoder* decoder, char **codec_info);

long ijkFfplayDecoder_getPropertyLong(IjkFfplayDecoder* decoder, int id, long default_value);

float ijkFfplayDecoder_getPropertyFloat(IjkFfplayDecoder* decoder, int id, float default_value);

int ijkFfplayDecoder_getMediaMeta(IjkFfplayDecoder* decoder, IjkMetadata* metadata);

//decoder_name: h264_cuvid, h264_qsv
int ijkFfplayDecoder_setHwDecoderName(IjkFfplayDecoder* decoder, const char* decoder_name);

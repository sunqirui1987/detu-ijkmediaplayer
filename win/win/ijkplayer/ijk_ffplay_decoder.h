//
//  ijk_ffplay_decoder.h
//  IJKMediaPlayer
//
//  Created by chao on 2017/6/12.
//  Copyright © 2017年 detu. All rights reserved.
//

#ifndef IJK_FFPLAYER_DECODER_H
#define IJK_FFPLAYER_DECODER_H

#include <stdio.h>

// media meta
#define k_IJKM_KEY_FORMAT         @"format"
#define k_IJKM_KEY_DURATION_US    @"duration_us"
#define k_IJKM_KEY_START_US       @"start_us"
#define k_IJKM_KEY_BITRATE        @"bitrate"

// stream meta
#define k_IJKM_KEY_TYPE           @"type"
#define k_IJKM_VAL_TYPE__VIDEO    @"video"
#define k_IJKM_VAL_TYPE__AUDIO    @"audio"
#define k_IJKM_VAL_TYPE__UNKNOWN  @"unknown"

#define k_IJKM_KEY_CODEC_NAME      @"codec_name"
#define k_IJKM_KEY_CODEC_PROFILE   @"codec_profile"
#define k_IJKM_KEY_CODEC_LONG_NAME @"codec_long_name"

// stream: video
#define k_IJKM_KEY_WIDTH          @"width"
#define k_IJKM_KEY_HEIGHT         @"height"
#define k_IJKM_KEY_FPS_NUM        @"fps_num"
#define k_IJKM_KEY_FPS_DEN        @"fps_den"
#define k_IJKM_KEY_TBR_NUM        @"tbr_num"
#define k_IJKM_KEY_TBR_DEN        @"tbr_den"
#define k_IJKM_KEY_SAR_NUM        @"sar_num"
#define k_IJKM_KEY_SAR_DEN        @"sar_den"
// stream: audio
#define k_IJKM_KEY_SAMPLE_RATE    @"sample_rate"
#define k_IJKM_KEY_CHANNEL_LAYOUT @"channel_layout"

#define kk_IJKM_KEY_STREAMS       @"streams"

typedef enum IJKLogLevel {
    k_IJK_LOG_UNKNOWN = 0,
    k_IJK_LOG_DEFAULT = 1,
    
    k_IJK_LOG_VERBOSE = 2,
    k_IJK_LOG_DEBUG   = 3,
    k_IJK_LOG_INFO    = 4,
    k_IJK_LOG_WARN    = 5,
    k_IJK_LOG_ERROR   = 6,
    k_IJK_LOG_FATAL   = 7,
    k_IJK_LOG_SILENT  = 8,
} IJKLogLevel;

typedef struct IjkFfplayDecoderCallBack {
    
}IjkFfplayDecoderCallBack;


struct IjkFfplayDecoder;

typedef struct IjkFfplayDecoder;

void ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* fileAbsolutePath);

void ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_start(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_reset(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_release(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec);

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder);

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float leftVolume, float rightVolume);

void ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, const char* key, long value);

void ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, const char* key, const char* value);

void ijkFfplayDecoder_setLogLevel(IjkFfplayDecoder* decoder, IJKLogLevel logLevel);

void ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, IjkFfplayDecoderCallBack* callBack);

#endif /* ijk_ffplay_decoder_h */

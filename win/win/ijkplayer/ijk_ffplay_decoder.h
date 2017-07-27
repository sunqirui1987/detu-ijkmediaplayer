//
//  ijk_ffplay_decoder.h
//  IJKMediaPlayer
//
//  Created by chao on 2017/6/12.
//  Copyright © 2017 detu. All rights reserved.
//

#ifndef IJK_FFPLAYER_DECODER_H
#define IJK_FFPLAYER_DECODER_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "detu_frame.h"

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

//ijk log level
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

//message state
typedef enum IjkMsgState{
	IJK_MSG_FLUSH                       = 0,
	IJK_MSG_ERROR                       = 100,     /* arg1 = error */
	IJK_MSG_PREPARED                    = 200,
	IJK_MSG_COMPLETED                   = 300,
	IJK_MSG_VIDEO_SIZE_CHANGED          = 400,     /* arg1 = width, arg2 = height */
	IJK_MSG_SAR_CHANGED                 = 401,     /* arg1 = sar.num, arg2 = sar.den */
	IJK_MSG_VIDEO_RENDERING_START       = 402,
	IJK_MSG_AUDIO_RENDERING_START       = 403,
	IJK_MSG_VIDEO_ROTATION_CHANGED      = 404,     /* arg1 = degree */
	IJK_MSG_BUFFERING_START             = 500,
	IJK_MSG_BUFFERING_END               = 501,
	IJK_MSG_BUFFERING_UPDATE            = 502,     /* arg1 = buffering head position in time, arg2 = minimum percent in time or bytes */
	IJK_MSG_BUFFERING_BYTES_UPDATE      = 503,     /* arg1 = cached data in bytes,            arg2 = high water mark */
	IJK_MSG_BUFFERING_TIME_UPDATE       = 504,     /* arg1 = cached duration in milliseconds, arg2 = high water mark */
	IJK_MSG_SEEK_COMPLETE               = 600,     /* arg1 = seek position,                   arg2 = error */
	IJK_MSG_PLAYBACK_STATE_CHANGED      = 700,
	IJK_MSG_TIMED_TEXT                  = 800,
	IJK_MSG_ACCURATE_SEEK_COMPLETE      = 900,     /* arg1 = current position*/
	IJK_MSG_VIDEO_DECODER_OPEN          = 10001,
}IjkMsgState;

typedef struct IjkFfplayDecoderCallBack {
	void (*func_get_frame)(void* opaque, sVideoFrame *frame_callback);
	void (*func_state_change)(void* opaque, IjkMsgState ijk_msgint, int arg1, int arg2);
}IjkFfplayDecoderCallBack;

struct IjkFfplayDecoder;
typedef struct IjkFfplayDecoder IjkFfplayDecoder;


/////////////////////////////////////////////////////////////////////////////////////////////////////////

void ijkFfplayDecoder_init(void);

void ijkFfplayDecoder_uninit(void);

IjkFfplayDecoder *ijkFfplayDecoder_create(void);

void ijkFfplayDecoder_setLogLevel(IJKLogLevel log_level);

void ijkFfplayDecoder_setDecoderCallBack(IjkFfplayDecoder* decoder, void* opaque, IjkFfplayDecoderCallBack* callback);

void ijkFfplayDecoder_setDataSource(IjkFfplayDecoder* decoder, const char* file_absolute_path);

void ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_start(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec);

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder);

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder);

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_release(IjkFfplayDecoder* decoder);

void ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float volume);

void ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, long value);

void ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, const char* value);

void ijkFfplayDecoder_getVideoCodecInfo(IjkFfplayDecoder* decoder, char **codec_info);

void ijkFfplayDecoder_getAudioCodecInfo(IjkFfplayDecoder* decoder, char **codec_info);

long ijkFfplayDecoder_getPropertyLong(IjkFfplayDecoder* decoder, int id, long default_value);

float ijkFfplayDecoder_getPropertyFloat(IjkFfplayDecoder* decoder, int id, float default_value);

#endif /* IJK_FFPLAYER_DECODER_H */

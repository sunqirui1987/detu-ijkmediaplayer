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
#import"IJKFFMoviePlayerController.h"

#define MAC_DECODER_NAME_SOFT "soft"
#define MAC_DECODER_NAME_VTB "h264_vtb"
#define MAC_MAX_DECODER_NAME_LENGTH 8

struct IjkFfplayDecoder {
    void* opaque;
    IJKFFMoviePlayerController* controller;
    char codecName[8];
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
    if(decoder == NULL || file_absolute_path == NULL || strlen(file_absolute_path) == 0) {
        return -1;
    }
    NSString* path = [[NSString alloc]initWithUTF8String:file_absolute_path];
    bool isVideoToolBox = true;
    if(strcmp(MAC_DECODER_NAME_SOFT, decoder->codecName) == 0) {
        isVideoToolBox = false;
    } else if(strcmp(MAC_DECODER_NAME_VTB, decoder->codecName) == 0) {
        isVideoToolBox = true;
    }
    decoder->controller = [[IJKFFMoviePlayerController alloc]initWithContentURLString:path withOptions:NULL isVideotoolbox:isVideoToolBox];
    //__weak IJKPlayerMovieDecoder* weakSelf = self;
    decoder->controller.displayFrameBlock = ^(SDL_VoutOverlay* overlay){
        if (overlay == NULL)return;
        [weakSelf.delegate movieF4DecoderDidDecodeFrameSDL:overlay index:0];
    };
    return 0;
}

int ijkFfplayDecoder_prepare(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller prepareToPlay];
    return 0;
}

int ijkFfplayDecoder_start(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller play];
    return 0;
}

int ijkFfplayDecoder_pause(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller pause];
    return 0;
}

int ijkFfplayDecoder_stop(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller stop];
    return 0;
}

int ijkFfplayDecoder_seekTo(IjkFfplayDecoder* decoder, long msec) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller setCurrentPlaybackTime:msec];
    return 0;
}

bool ijkFfplayDecoder_isPlaying(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    return [decoder->controller isPlaying];
}

long ijkFfplayDecoder_getCurrentPosition(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    return decoder->controller.currentPlaybackTime;
}

long ijkFfplayDecoder_getDuration(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    return decoder->controller.duration;
}

int ijkFfplayDecoder_release(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller shutdown];
    return 0;
}

int ijkFfplayDecoder_setVolume(IjkFfplayDecoder* decoder, float volume) {
    if(decoder == NULL) {
        return -1;
    }
    [decoder->controller setVolume:volume];
    return 0;
}

float ijkFfplayDecoder_getVolume(IjkFfplayDecoder* decoder) {
    if(decoder == NULL) {
        return -1;
    }
    return [decoder->controller getVolume];
}

int ijkFfplayDecoder_setOptionLongValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, long value) {
    if(decoder == NULL) {
        return -1;
    }
//    [decoder->controller setOptionValue:value forKey:[NSString stringWithUTF8String:key] ofCategory:<#(IJKFFOptionCategory)#>];
    return 0;
}

int ijkFfplayDecoder_setOptionStringValue(IjkFfplayDecoder* decoder, int opt_category, const char* key, const char* value) {
    if(decoder == NULL) {
        return -1;
    }
    return 0;
}

int ijkFfplayDecoder_getVideoCodecInfo(IjkFfplayDecoder* decoder, char **codec_info) {
    if(decoder == NULL) {
        return -1;
    }
    return 0;
}

int ijkFfplayDecoder_getAudioCodecInfo(IjkFfplayDecoder* decoder, char **codec_info) {
    if(decoder == NULL) {
        return -1;
    }
    return 0;
}

long ijkFfplayDecoder_getPropertyLong(IjkFfplayDecoder* decoder, int optionId, long default_value) {
    if(decoder == NULL) {
        return -1;
    }
    return 0l;
}

float ijkFfplayDecoder_getPropertyFloat(IjkFfplayDecoder* decoder, int optionId, float default_value) {
    if(decoder == NULL) {
        return -1;
    }
    return 0.0;
}

int ijkFfplayDecoder_getMediaMeta(IjkFfplayDecoder* decoder, IjkMetadata* metadata) {
    if(decoder == NULL) {
        return -1;
    }
    return 0;
}

//decoder_name: h264_vtb
int ijkFfplayDecoder_setHwDecoderName(IjkFfplayDecoder* decoder, const char* decoder_name) {
    if(decoder == NULL) {
        return -1;
    }
    if(decoder_name == NULL) {
        decoder_name = MAC_DECODER_NAME_SOFT;
    }
    int length = strlen(decoder_name);
    if(length > MAC_MAX_DECODER_NAME_LENGTH) {
        return -1;
    }
    memset(decoder->codecName, 0, sizeof(decoder->codecName));
    strcpy(decoder->codecName, decoder_name);
    return 0;
}

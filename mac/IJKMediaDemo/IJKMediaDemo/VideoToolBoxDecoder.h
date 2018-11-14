//
//  FfmpegDecoder.hpp
//  MutilRtspPlay_Lib
//
//  Created by chao on 2017/9/13.
//  Copyright © 2017年 detu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import "H264Utils.h"

#include <libavformat/avformat.h>
#include<libavutil/frame.h>

#define DECODE_MAX_FRAME_COUNT 3

typedef struct DecodeFrame {
    int width;
    int height;
    uint8_t data[DECODE_MAX_FRAME_COUNT];
    int lineSize[DECODE_MAX_FRAME_COUNT];
    void* opaque;
    enum AVPixelFormat format;
    int64_t pts;
    bool canRelease;
} DecodeFrame;

@protocol VideoToolBoxDecoderDelegate <NSObject>

@required
-(void)videoToolBoxDecoderFrame:(DecodeFrame*) frame identity:(int) identityId;
@end

@interface VideoToolBoxDecoder : NSObject

@property(nonatomic, weak)id<VideoToolBoxDecoderDelegate> delegate;

-(void)init:(AVCodecContext*) context :(int) identityId;

-(void)close;
-(bool)sendPacket:(AVPacket*) packet;
-(void)unrefFrame:(DecodeFrame*) frame;

@end


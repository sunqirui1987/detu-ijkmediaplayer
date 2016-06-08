//
//  IJKPlayerMovieDecoder.h
//  IJKMediaDemo
//
//  Created by qiruisun on 16/5/20.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define MOVIE_DECODER_TARGET_IOS 1
#import <AssetsLibrary/AssetsLibrary.h>
#else
#define MOVIE_DECODER_TARGET_OSX 1
#endif
#import <CoreVideo/CVPixelBuffer.h>
#import <UIKit/UIKit.h>
#include "ijksdl.h"
#include "ijksdl_vout_overlay_videotoolbox.h"

// BT.709, which is the standard for HDTV.
static const GLfloat kColorConversion709[] = {
    1.164,  1.164,  1.164,
    0.0,   -0.213,  2.112,
    1.793, -0.533,  0.0,
};

// BT.601, which is the standard for SDTV.
static const GLfloat kColorConversion601[] = {
    1.164,  1.164, 1.164,
    0.0,   -0.392, 2.017,
    1.596, -0.813,   0.0,
};



typedef enum {
    MOVICE_STATE_PLAYING,
    MOVICE_STATE_STOP,
    MOVICE_STATE_PAUSE,
    MOVICE_STATE_BUFFER_EMPTY,
    MOVICE_STATE_START_SEEK,
    MOVICE_STATE_FAILED,
    MOVICE_STATE_READYTOPALY,
    MOVICE_STATE_UNKNOWN
    
}MovieDecoderPlayItemState;

@protocol MovieDecoderDelegate <NSObject>

@required

-(void)movieDecoderDidFinishDecoding;
-(void)movieDecoderDidSeeked;
-(void)movieDecoderError:(NSError *)error;
-(void)moviceDecoderPlayItemState:(MovieDecoderPlayItemState)state;

@optional
-(void)movieDecoderDidDecodeFrame:(CVPixelBufferRef)buffer;
-(void)movieDecoderDidDecodeFrameBuffer:(void*)buffer width:(int)width height:(int)height channel:(int)channel;
-(void)movieDecoderDidDecodeFrameSDL:(SDL_VoutOverlay*)frame;

@end



@interface IJKPlayerMovieDecoder : NSObject

@property (nonatomic,readonly)   float duration;
@property (nonatomic,readonly)   float bufferedTime;
@property int isreplay;
@property bool is_hardware;
@property (nonatomic,assign)   double currentTime;
@property (nonatomic,weak)   id<MovieDecoderDelegate> delegate;

+(id)movieDecoder;
+(id)movieDecoderWithMovie:(NSString*)path;
+(id)movieDecoderWithMovie:(NSString*)path isHardWare:(BOOL)isHardWare;
-(id)initWithMovie:(NSString*)path;
-(BOOL)loadMovie:(NSString*)path;
-(void)captureNext;
-(void)start;
-(void)pause;
-(void)stop;
-(void)cleargc;

-(NSString*)getDescriptMedta:(NSString *)key;
@end
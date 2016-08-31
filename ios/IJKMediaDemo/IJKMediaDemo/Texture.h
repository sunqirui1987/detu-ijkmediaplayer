//
//  Texture.m
//  IJKMediaDemo
//
//  Created by qiruisun on 16/5/20.
//  Copyright © 2016年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "IJKPlayerMovieDecoder.h"

@interface Texture : NSObject{
@public
    
    //    GLuint textures[2];
    //    GLint _uniformSamplers[2];
    //
    //    CVOpenGLESTextureCacheRef textureCache;
    //    CVOpenGLESTextureRef      cvTexturesRef[2];
    //    GLint _uniformIsHardWare;
    
    
    const GLfloat *_preferredConversion;
    GLuint _textures[3];
    GLint _uniformSamplers[3];
    GLint _uniform[1];
    
    
}

@property(assign,readonly) float xRange;
@property(assign,readonly) float yRange;

-(id)initWithContext:(EAGLContext*)context;

-(void)updateWithBuffer:(CVPixelBufferRef)buf;
-(void)updateWithDataBuffer:(void *)data ww:(int)ww hh:(int)hh channel:(int)channel;
-(void)updateWithImage:(UIImage *)img;

-(void)updateWithFrameSDL:(SDL_VoutOverlay*)overlay;
-(void)updateWithFrameBuf:(uint8_t *)frame:(int)w:(int)h;

-(void)bind;
@end
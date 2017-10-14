 /*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 The OpenGLRenderer class creates and draws objects.
  Most of the code is OS independent.
 */
#include "glUtil.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

typedef enum RcColorFormat {
    FMT_RGBA,
    FMT_YUV420P,
    FMT_NV12,
    FMT_VTB
}RcColorFormat;

typedef struct RcFrame {
    uint8_t *data[3];
    int width;
    int height;
    int linesize[3];
    int planes;
    RcColorFormat format;
} RcFrame;

@interface OpenGLRenderer : NSObject 

@property (nonatomic) GLuint defaultFBOName;

- (instancetype) initWithDefaultFBO: (GLuint) defaultFBOName;
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void) dealloc;

- (void)setImage:(RcFrame*)overlay;

@end

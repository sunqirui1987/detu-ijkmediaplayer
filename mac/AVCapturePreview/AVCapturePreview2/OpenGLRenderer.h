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
#include "ijksdl_vout.h"

@interface OpenGLRenderer : NSObject 

@property (nonatomic) GLuint defaultFBOName;

- (instancetype) initWithDefaultFBO: (GLuint) defaultFBOName;
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void) dealloc;

- (void)setImage:(SDL_VoutOverlay*)overlay;

@end

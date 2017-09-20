//
//  GlRender.h
//  IJKMediaDemo
//
//  Created by chao on 2017/6/24.
//  Copyright © 2017年 detu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "glUtil.h"
#include "ijksdl_vout.h"

@interface GlRender : NSObject{
    SDL_VoutOverlay* overLay;
}

@property (nonatomic) GLuint defaultFBOName;

- (void) initWithDefaultFBO: (GLuint) defaultFBOName;
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void) setOverlay:(SDL_VoutOverlay*)frameOverlay;
- (void) dealloc;


@end

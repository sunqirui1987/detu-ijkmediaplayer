//
//  GLESRenderer.m
//  PanoPlayer
//
//  Created by qiruisun on 15/11/12.
//  Copyright © 2015年 apple. All rights reserved.
//

#import "GLESRenderer.h"

@implementation GLESRenderer

- (BOOL)createFramebuffer:(CAEAGLLayer*)layer{
    return true;
}
- (void)destroyFramebuffer{
}
- (void)setFramebuffer:(CAEAGLLayer*)layer{
}
- (BOOL)presentFramebuffer{
    return true;
}
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file{
    return true;
}
@end
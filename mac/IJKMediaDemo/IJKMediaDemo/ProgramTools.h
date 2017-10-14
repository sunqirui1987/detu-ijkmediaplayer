//
//  ProgramTools.h
//  IJKMediaDemo
//
//  Created by chao on 2017/6/20.
//  Copyright © 2017年 detu. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>

@interface ProgramTools : NSObject

-(GLuint)compileProgram:(NSString*)fileName;

-(GLuint)compileProgram:(const char *)vertextStr frag:(const char *)fragStr;

@end

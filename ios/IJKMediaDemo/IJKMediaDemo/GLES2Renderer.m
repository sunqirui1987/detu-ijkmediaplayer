//
//  GLES2Renderer.m
//  IJKMediaDemo
//
//  Created by qiruisun on 16/5/20.
//  Copyright © 2016年 bilibili. All rights reserved.
//

//
//  GLES1Renderer.m
//  PanoPlayer
//
//  Created by qiruisun on 15/10/21.
//  Copyright © 2015年 apple. All rights reserved.
//

#import "GLES2Renderer.h"
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <GLKit/GLKMath.h>



@interface GLES2Renderer()
{
    
    
    GLuint          _framebuffer;
    GLuint          _renderbuffer;
    GLint           _backingWidth;
    GLint           _backingHeight;
    
    GLuint programHandler;
    
}
@end

@implementation GLES2Renderer

- (id)init
{
    if((self =[super init]))
    {
        [self initRenderer];
    }
    return self;
}

- (void)initRenderer{
    
    
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    self.context = [[EAGLContext alloc] initWithAPI:api];
    if (!self.context) {
        NSLog(@"Failed to initialize OpenGLES 2.0 context");
    }
    
    if (![EAGLContext setCurrentContext:self.context]) {
        NSLog(@"Failed to set current OpenGL context");
    }
    
    
}


- (BOOL)createFramebuffer:(CAEAGLLayer*)layer {
    
    [self destroyFramebuffer];
    [EAGLContext setCurrentContext:self.context];
    
    
    glGenFramebuffers(1, &_framebuffer);
    glGenRenderbuffers(1, &_renderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
    [self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x\n", status);
        return NO;
    }
    
    GLenum glError = glGetError();
    if (GL_NO_ERROR != glError) {
        NSLog(@"failed to setup GL %x\n", glError);
        return NO;
    }
    
    return YES;
}

- (void)destroyFramebuffer {
    if (self.context) {
        [EAGLContext setCurrentContext:self.context];
        
        
        if (_framebuffer) {
            glDeleteFramebuffers(1, &_framebuffer);
            _framebuffer = 0;
        }
        
        if (_renderbuffer) {
            glDeleteRenderbuffers(1, &_renderbuffer);
            _renderbuffer = 0;
        }
        
        //        if(msaaFramebuffer)
        //        {
        //            glDeleteFramebuffers(1, &msaaFramebuffer);
        //            msaaFramebuffer = 0;
        //        }
        //
        //        if(msaaRenderBuffer)
        //        {
        //            glDeleteRenderbuffers(1, &msaaRenderBuffer);
        //            msaaRenderBuffer = 0;
        //        }
        //
        //        if(msaaDepthBuffer)
        //        {
        //            glDeleteRenderbuffers(1, &msaaDepthBuffer);
        //            msaaDepthBuffer = 0;
        //        }
    }
    
}



- (void)setFramebuffer:(CAEAGLLayer*)layer {
    
    if (self.context) {
        [EAGLContext setCurrentContext:self.context];
        
        if (!_framebuffer)
            [self createFramebuffer:layer];
        
        
        
        //  glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer); //Bind MSAA
        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        
        
        
        // Use shader program
        glUseProgram(programHandler);
        
        
    }
}

- (BOOL)presentFramebuffer {
    BOOL success = FALSE;
    if (self.context) {
        [EAGLContext setCurrentContext:self.context];
        
        //        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, _framebuffer);
        //        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, _renderbuffer);
        //        // Call a resolve to combine buffers
        //        glResolveMultisampleFramebufferAPPLE();
        //        // Present final image to screen
        //        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
        //        glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
        success = [self.context presentRenderbuffer:GL_RENDERBUFFER];
    }
    
    return success;
}



- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source)
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*shader);
        return FALSE;
    }
    
    return TRUE;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (GLuint)loadShaders:(NSString*)sharder
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program
    programHandler = glCreateProgram();
    
    // Create and compile vertex shader
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:sharder ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:sharder ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname])
    {
        NSLog(@"Failed to compile fragment shader");
        return FALSE;
    }
    
    // Attach vertex shader to program
    glAttachShader(programHandler, vertShader);
    
    // Attach fragment shader to program
    glAttachShader(programHandler, fragShader);
    
    
    // Link program
    if (![self linkProgram:programHandler])
    {
        NSLog(@"Failed to link program: %d", programHandler);
        
        if (vertShader)
        {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (programHandler)
        {
            glDeleteProgram(programHandler);
            programHandler = 0;
        }
        
        return 0;
    }
    
    
    
    // Release vertex and fragment shaders
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    return programHandler;
}


- (int)getGLWidth{
    return _backingWidth;
}

- (int)getGLHeight{
    return _backingHeight;
}



@end
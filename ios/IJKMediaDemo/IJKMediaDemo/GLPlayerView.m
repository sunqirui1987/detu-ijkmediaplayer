//
//  GLView.m
//  PanoPlayer
//
//  Created by apple on 15/7/6.
//  Copyright (c) 2015年 apple. All rights reserved.
//
//This file is part of MyVideoPlayer.

#import "GLPlayerView.h"
#import "GLES2Renderer.h"

#import <GLKit/GLKMath.h>
#import "Texture.h"



#define TOUCHMODE_INIT 1
#define TOUCHMODE_ZOOM 2
#define TOUCHMODE_DRAG 3

#define ACCSPEED 0.001

#define OPENGL_SCALE    [[UIScreen mainScreen] scale]

@interface GLPlayerView()
{
    
    GLKMatrix4 projectionMatrix;
    GLKMatrix4 modelMatrix;
    GLKMatrix4 viewMatrix;
    
    GLuint mProgramHandle;
    GLuint mMVPMatrixHandle;
    GLuint mTextureUniformHandle;
    GLuint mPositionHandle;
    GLuint mTextureCoordinateHandle;
    GLuint misforwardHandle;
    
    
    Texture *texture;
    float w_h_rate;
    
    
    GLint _uniformSamplers[3];
    GLint _uniform[1];
}
@end

@implementation GLPlayerView

// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

-(id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) {
        [self _initLayer];
    }
    return self;
}

-(id)initWithCoder:(NSCoder *)aDecoder{
    if ((self = [super initWithCoder:aDecoder])) {
        [self _initLayer];
    }
    return self;
}

-(void)_initLayer{
    
    self.layer.opaque = YES;
    self.layer.contentsScale = OPENGL_SCALE;
    /*
     ((CAEAGLLayer *)self.layer).drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGB565,kEAGLDrawablePropertyColorFormat, nil];
     
     */
    //    ((CAEAGLLayer *)self.layer).drawableProperties = @{
    //                                                       kEAGLDrawablePropertyRetainedBacking: [NSNumber numberWithBool:NO],
    //                                                       kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
    //                                                       };
    _renderer = [[GLES2Renderer alloc]init];
    texture = [[Texture alloc]init];
    
    
    
    mProgramHandle = [_renderer loadShaders:@"sphere"];
    
    mMVPMatrixHandle = glGetUniformLocation(mProgramHandle, "u_MVPMatrix");
    mTextureUniformHandle = glGetUniformLocation(mProgramHandle, "u_Texture");
    mPositionHandle = glGetAttribLocation(mProgramHandle, "a_Position");
    mTextureCoordinateHandle = glGetAttribLocation(mProgramHandle, "a_TexCoordinate");
    misforwardHandle = glGetUniformLocation(mProgramHandle,
                                            "forward");
    
    
    //    texture->_uniformSamplers[0] = glGetUniformLocation(mProgramHandle, "SamplerY1");
    //    texture->_uniformSamplers[1] = glGetUniformLocation(mProgramHandle, "SamplerUV");
    //    texture->_uniform[0] = glGetUniformLocation(mProgramHandle, "colorConversionMatrix");
    //    texture->_uniformIsHardWare = glGetUniformLocation(mProgramHandle,
    //                                            "ishardware");
    
    texture->_uniformSamplers[0] = glGetUniformLocation(mProgramHandle, "SamplerY");
    texture->_uniformSamplers[1] = glGetUniformLocation(mProgramHandle, "SamplerU");
    texture->_uniformSamplers[2] = glGetUniformLocation(mProgramHandle, "SamplerV");
    texture->_uniform[0] = glGetUniformLocation(mProgramHandle, "colorConversionMatrix");
    
    
    
}



- (void)checkGLError:(BOOL)visibleCheck {
    GLenum error = glGetError();
    
    switch (error) {
        case GL_INVALID_ENUM:
            NSLog(@"GL Error: Enum argument is out of range");
            break;
        case GL_INVALID_VALUE:
            NSLog(@"GL Error: Numeric value is out of range");
            break;
        case GL_INVALID_OPERATION:
            NSLog(@"GL Error: Operation illegal in current state");
            break;
        case GL_OUT_OF_MEMORY:
            NSLog(@"GL Error: Not enough memory to execute command");
            break;
        case GL_NO_ERROR:
            if (visibleCheck) {
                NSLog(@"No GL Error");
            }
            break;
        default:
            NSLog(@"Unknown GL Error %d",error);
            break;
    }
}

-(void)setFrameSDL:(SDL_VoutOverlay*)frame{
    
    int width = frame->w;
    int height = frame->h;
    w_h_rate = width * 1.0f / height;
    [texture updateWithFrameSDL:frame];
    
}


-(void)render{
    
    if(texture->_textures == NULL || w_h_rate == 0){
        return;
    }
    [_renderer setFramebuffer:(CAEAGLLayer*)self.layer];
    
    
    
    int view_w = self.frame.size.width;
    int view_h = self.frame.size.height;
    
    glViewport(0,0,view_w*OPENGL_SCALE,view_h*OPENGL_SCALE);
    
    
    
    
    modelMatrix = GLKMatrix4Identity;
    projectionMatrix =GLKMatrix4Identity;
    
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture->_textures[i]);
        glUniform1i(texture->_uniformSamplers[i], i);
    }
    
    glUniformMatrix3fv(texture->_uniform[0], 1, GL_FALSE, texture->_preferredConversion);
    
    
    glUniform1i(mTextureUniformHandle, 0);
    glEnableVertexAttribArray(mPositionHandle);
    glEnableVertexAttribArray(mTextureCoordinateHandle);
    
    
    GLKMatrix4 mvPMatrix =  GLKMatrix4Multiply(projectionMatrix,modelMatrix);
    glUniformMatrix4fv(mMVPMatrixHandle, 1, false, mvPMatrix.m);
    
    
    
    
    
    float xr = 1.0,yr=1.0;
    if (texture != nil) {
        xr = texture.xRange;
        yr = texture.yRange;
    }
    
    float x=1.0;
    float y=1.0;
    if(view_w / w_h_rate < view_h){
        x = 1.0;
        y = 1.0f/w_h_rate * view_w / view_h ;
    }else{
        y = 1.0;
        x = w_h_rate * view_h / view_w ;
    }
    
    
    float s_plane_points[8] = {
        -x,y,
        x,y,
        
        -x,-y,
        x,-y
    };
    float s_plane_spriteTexcoords[8] = {
        0,0,
        xr,0,
        
        0,yr,
        xr,yr
    };
    
    
    
    glVertexAttribPointer(mPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &s_plane_points);
    glVertexAttribPointer(mTextureCoordinateHandle, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &s_plane_spriteTexcoords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0,4);
    
    
    
    
    [_renderer presentFramebuffer];
    
    [self checkGLError:NO];
}




-(void)dealloc{
    [_renderer destroyFramebuffer];
}

- (UIImage *)getScreenShot {
    CGSize size = self.bounds.size;
    
    UIGraphicsBeginImageContextWithOptions(size, NO, [UIScreen mainScreen].scale);
    
    CGRect rec = CGRectMake(self.frame.origin.x, self.frame.origin.y, self.bounds.size.width, self.bounds.size.height);
    [self drawViewHierarchyInRect:rec afterScreenUpdates:YES];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

@end
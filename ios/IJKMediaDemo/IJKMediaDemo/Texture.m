//
//  Texture.m
//  PanoPlayer
//
//  Created by apple on 15/7/1.
//  Copyright (c) 2015年 apple. All rights reserved.
//

#import "Texture.h"
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


@interface Texture()
{
    int width,height,texWidth,texHeight,channel;
    GLuint textureId;
}
@end

@implementation Texture

-(instancetype)init{
    self=[super init];
    [self clear];
    return self;
}

-(id)initWithContext:(EAGLContext*)context{
    self=[super init];
    [self clear];
    return self;
}

-(void)clear{
    width=0;
    height=0;
    channel=0;
    textureId=0;
    [self delete];
}

-(void)updateWithImage:(UIImage *)img{
    
    if (!img) {
        [self clear];
        return;
    }
    
    CGSize size= img.size;// [self _getTextureSize:img.size];
    int w,h,c;
    w=size.width;
    h=size.height;
    c=4;
    
    CGColorSpaceRef colorspace=CGColorSpaceCreateDeviceRGB();
    
    int bytesPerRow=c*w;
    int bitsPerComponent = 8;
    
    void *data=malloc(w*h*c);
    
    CGContextRef cgcnt = CGBitmapContextCreate(data,
                                               w,
                                               h,
                                               bitsPerComponent,
                                               bytesPerRow,
                                               colorspace,
                                               kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    
    CGColorSpaceRelease(colorspace);
    
    //将图像写入一个矩形
    CGRect therect = CGRectMake(0, 0, w, h);
    CGContextDrawImage(cgcnt, therect, img.CGImage);
    //    释放资源
    CGContextRelease(cgcnt);
    
    
    if (data != nil) {
        [self create:data width:w heihgt:h channel:c];
        
        free(data);
    }
    
}

-(void)updateWithDataBuffer:(void *)data ww:(int)ww hh:(int)hh channel:(int)channel{
    
    int w,h,c;
    w=ww;
    h=hh;
    
    c=channel;
    
    
    if (data != nil) {
        [self create:data width:w heihgt:h channel:c];
        
        //free(buf);
    }
}

-(void)updateWithBuffer:(CVPixelBufferRef)buf{
    if (!buf) {
        [self clear];
        return;
    }
    
    CVPixelBufferLockBaseAddress(buf, 0);
    int w,h,c;
    w= (int)CVPixelBufferGetWidth(buf);
    h = (int)CVPixelBufferGetHeight(buf);
    void* data=CVPixelBufferGetBaseAddress(buf);
    OSType type=CVPixelBufferGetPixelFormatType(buf);
    if (type==kCVPixelFormatType_24RGB) {
        c=3;
    }else{
        c=4;
    }
    CVPixelBufferUnlockBaseAddress(buf, 0);
    
    [self create:data width:w heihgt:h channel:c];
}

-(CGSize)_getTextureSize:(CGSize)orgSize{
    int h=2;
    while (h<orgSize.height) {
        h*=2;
    }
    int w=2;
    while (w<orgSize.width) {
        w*=2;
    }
    return CGSizeMake(w, h);
}

-(void)delete
{
    glDeleteTextures(1,&textureId);
    textureId=0;
    
    for (int i = 0; i < 2; ++i) {
        if (hard_cvTexturesRef[i]) {
            CFRelease(hard_cvTexturesRef[i]);
            hard_cvTexturesRef[i] = 0;
        }
    }
    if(hard_textureCache){
        CFRelease(hard_textureCache);
        hard_textureCache = nil;
    }
    
    if (_textures[0])
        glDeleteTextures(3, _textures);
}

-(void)bind{
    glBindTexture(GL_TEXTURE_2D, textureId);
}


-(void)create:(void*)data width:(int)w heihgt:(int)h channel:(int)c
{
    
    height=h;
    width=w;
    channel=c;
    _xRange=1.0f;
    _yRange=1.0f;
    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (textureId == 0) {
        glGenTextures(1, &textureId);
    }
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    if (channel==2) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,data);
    }else if(channel==3){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,data);
    }else if(channel==4){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,data);
    }
}

-(void)create2:(void*)data width:(int)w heihgt:(int)h channel:(int)c
{
    
    bool createNew=(w>width||h>height||c!=channel);
    if(createNew){
        CGSize size=[self _getTextureSize:CGSizeMake(w, h)];
        height=size.height;
        width=size.width;
        channel=c;
    }
    _xRange=w*1.0f/width;
    _yRange=h*1.0f/height;
    
    
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //  glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (textureId == 0) {
        glGenTextures(1, &textureId);
    }
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    if (createNew) {
        void *tex=malloc(width*height*channel);
        
        if (channel==2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,tex);
        }else if(channel==3){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,tex);
        }else if(channel==4){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,tex);
        }
        free(tex);
    }
    
    if (channel==2) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,data);
    }else if(channel==3){
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE,data);
    }else if(channel==4){
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,data);
    }
}

-(void)updateWithFrameSDL:(SDL_VoutOverlay*)overlay{
    switch (overlay->format) {
        case SDL_FCC__VTB:
//            @try {
                [self hardPrepare:overlay];
//            } @catch (NSException *exception) {
//                NSLog(@"%@",exception);
//            } @finally {
//                
//            }
    
            break;
        default:
            [self softPrepare:overlay];
            break;
    }
}

-(void)updateWithFrameBuf:(uint8_t*)frame:(int)w:(int)h{
    [self softPrepare:frame:w:h];
}

-(void)hardPrepare:(SDL_VoutOverlay*)overlay{
    assert(overlay->format == SDL_FCC__VTB);
    assert(overlay->planes == 2);

    CVPixelBufferRef pixel_buffer = SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(overlay);
    if (!pixel_buffer) {
        NSLog(@"nil pixelBuffer in overlay\n");
        return;
    }
    
    
    
    
    for (int i = 0; i < 2; ++i) {
        if (hard_cvTexturesRef[i]) {
            CFRelease(hard_cvTexturesRef[i]);
            hard_cvTexturesRef[i] = nil;
        }
    }
    
    // Periodic texture cache flush every frame
    if (hard_textureCache)
        CVOpenGLESTextureCacheFlush(hard_textureCache, 0);
    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _preferredConversion = kColorConversion601;
    
    
//    CVReturn result = CVPixelBufferLockBaseAddress(pixel_buffer, 0);
//    NSLog(@"pixelBuffer result = %d",result);
    
    GLsizei frame_width  = (GLsizei)CVPixelBufferGetWidth(pixel_buffer);
    GLsizei frame_height = (GLsizei)CVPixelBufferGetHeight(pixel_buffer);
    
    _xRange=1.0f;
    _yRange=1.0f;
    width = (int)frame_width;
    height = (int)frame_height;
    
//    [self checkGLError:YES];
    
    glActiveTexture(GL_TEXTURE0);
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                 hard_textureCache,
                                                 pixel_buffer,
                                                 NULL,
                                                 GL_TEXTURE_2D,
                                                 GL_RED_EXT,
                                                 (GLsizei)frame_width,
                                                 (GLsizei)frame_height,
                                                 GL_RED_EXT,
                                                 GL_UNSIGNED_BYTE,
                                                 0,
                                                 &hard_cvTexturesRef[0]);
    if (err) {
        NSLog(@"Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
    }

    glBindTexture(CVOpenGLESTextureGetTarget(hard_cvTexturesRef[0]), CVOpenGLESTextureGetName(hard_cvTexturesRef[0]));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
 //   [self checkGLError:YES];
    
    
    glActiveTexture(GL_TEXTURE1);
    err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                 hard_textureCache,
                                                 pixel_buffer,
                                                 NULL,
                                                 GL_TEXTURE_2D,
                                                 GL_RG_EXT,
                                                 (GLsizei)frame_width / 2,
                                                 (GLsizei)frame_height / 2,
                                                 GL_RG_EXT,
                                                 GL_UNSIGNED_BYTE,
                                                 1,
                                                 &hard_cvTexturesRef[1]);
    if (err) {
        NSLog(@"Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err);
    }

    
    glBindTexture(CVOpenGLESTextureGetTarget(hard_cvTexturesRef[1]), CVOpenGLESTextureGetName(hard_cvTexturesRef[1]));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    
//    if (pixel_buffer != NULL) {  // 不加，会导致纹理没释放，id 不断上升
//        CFRelease(pixel_buffer);
//    }
}

-(void)softPrepare:(SDL_VoutOverlay*)overlay{
    assert(overlay->planes);
    assert(overlay->format == SDL_FCC_I420);
    assert(overlay->planes == 3);
    assert(overlay->planes == 3);
    // assert(yuvFrame.luma.length == yuvFrame.width * yuvFrame.height);
    // assert(yuvFrame.chromaB.length == (yuvFrame.width * yuvFrame.height) / 4);
    // assert(yuvFrame.chromaR.length == (yuvFrame.width * yuvFrame.height) / 4);
    
    const NSUInteger frameHeight = overlay->h;
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if (0 == _textures[0])
        glGenTextures(3, _textures);
    
    _preferredConversion = kColorConversion709;
    
    const UInt8 *pixels[3] = { overlay->pixels[0], overlay->pixels[1], overlay->pixels[2] };
    const NSUInteger widths[3]  = { overlay->pitches[0], overlay->pitches[1], overlay->pitches[2] };
    const NSUInteger heights[3] = { frameHeight, frameHeight / 2, frameHeight / 2 };
    
    
    //width = (int)widths[0];
    //height = (int)heights[0];
    
    _xRange=1.0f;
    _yRange=1.0f;
    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (int i = 0; i < 3; ++i) {
        
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     (int)widths[i],
                     (int)heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     pixels[i]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

-(void)softPrepare:(uint8_t*)frame:(int)w:(int)h{
    assert(frame);
    // assert(yuvFrame.luma.length == yuvFrame.width * yuvFrame.height);
    // assert(yuvFrame.chromaB.length == (yuvFrame.width * yuvFrame.height) / 4);
    // assert(yuvFrame.chromaR.length == (yuvFrame.width * yuvFrame.height) / 4);
    
    const NSUInteger frameHeight = h;
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    if (0 == _textures[0])
        glGenTextures(3, _textures);
    
    _preferredConversion = kColorConversion709;
    
    const UInt8 *pixels[3];
    
    const char *y = (const char *)frame;
    const char *u = y + w * h;
    const char *v = u + w * h / 4;
    
    pixels[0] = y;
    pixels[1] = u;
    pixels[2] = v;
    const NSUInteger widths[3]  = { w, w/2, w/2 };
    const NSUInteger heights[3] = { h, h / 2, h / 2 };
    
    
    width = (int)widths[0];
    height = (int)heights[0];
    
    _xRange=1.0f;
    _yRange=1.0f;
    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    for (int i = 0; i < 3; ++i) {
        
        glBindTexture(GL_TEXTURE_2D, _textures[i]);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     (int)widths[i],
                     (int)heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     pixels[i]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

-(void)dealloc{
    [self delete];
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


@end

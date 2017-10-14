/*
 Copyright (C) 2015 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 The OpenGLRenderer class creates and draws objects.
  Most of the code is OS independent.
 */

#import "OpenGLRenderer.h"
#include "glUtil.h"
#import "ProgramTools.h"

#define RC_GLES_STRINGIZE(x) #x

//标准顶点坐标
static const char FLAT_VSH_STR[] = RC_GLES_STRINGIZE(
            attribute vec4 a_Position;
            attribute vec2 a_TexCoordinate;
            varying vec2 v_TexCoordinate;
            void main()
            {
             v_TexCoordinate = a_TexCoordinate;
             gl_Position = a_Position;
            }
                                                    );


static const char FLAT_FSH_YUV420P_STR[] = RC_GLES_STRINGIZE(
                                                             uniform sampler2D sample0;
                                                             uniform sampler2D sample1;
                                                             uniform sampler2D sample2;
                                                             const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);
                                                             const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
                                                             const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
                                                             const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);
                                                             varying vec2 v_TexCoordinate;
                                                             void main() {
                                                                 vec3 yuv;
                                                                 vec3 rgb;
                                                                 yuv.x = texture2D(sample0, v_TexCoordinate).r;
                                                                 yuv.y = texture2D(sample1, v_TexCoordinate).g;
                                                                 yuv.z = texture2D(sample2, v_TexCoordinate).b;
                                                                 yuv += offset;
                                                                 rgb.r = dot(yuv, Rcoeff);
                                                                 rgb.g = dot(yuv, Gcoeff);
                                                                 rgb.b = dot(yuv, Bcoeff);
                                                                 gl_FragColor = vec4(rgb, 1.0);
                                                             }
                                                             );

static const char FLAT_FSH_NV12_STR[] = RC_GLES_STRINGIZE(
                                                          uniform sampler2D sample0;
                                                          uniform sampler2D sample1;
                                                          const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);
                                                          const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
                                                          const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
                                                          const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);
                                                          varying vec2 v_TexCoordinate;
                                                          void main() {
                                                              vec3 yuv;
                                                              vec3 rgb;
                                                              yuv.x = texture2D(sample0, v_TexCoordinate).r;
                                                              yuv.yz = texture2D(sample1, v_TexCoordinate).ra;
                                                              yuv += offset;
                                                              rgb.r = dot(yuv, Rcoeff);
                                                              rgb.g = dot(yuv, Gcoeff);
                                                              rgb.b = dot(yuv, Bcoeff);
                                                              gl_FragColor = vec4(rgb, 1.0);
                                                          }
                                                          );

@interface OpenGLRenderer ()
{
    GLuint _defaultFBOName;
    GLuint _viewWidth;
    GLuint _viewHeight;
    GLuint textures[3];
    GLuint uniformSamplers[3];
    GLuint aPostionLocation;
    GLuint aTextureCoordLocation;
    GLuint mMVPMatrixHandle;
    GLuint program;
    RcFrame* overLay;
    int index;
    NSLock* renderLock;
}
@end

@implementation OpenGLRenderer


- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height
{
    if(renderLock == nil) {
        renderLock = [[NSLock alloc]init];
    }
	glViewport(0, 0, width, height);

	_viewWidth = width;
	_viewHeight = height;
}

-(void)checkGLError {
    
    GLenum error = glGetError();
    
    switch (error) {
        case GL_INVALID_ENUM:
            NSLog(@"GL Error: Enum argument is out of range \r\n");
            break;
        case GL_INVALID_VALUE:
            NSLog(@"GL Error: Numeric value is out of range \r\n");
            break;
        case GL_INVALID_OPERATION:
            NSLog(@"GL Error: Operation illegal in current state \r\n");
            break;
        case GL_OUT_OF_MEMORY:
            NSLog(@"GL Error: Not enough memory to execute command \r\n");
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            NSLog(@"GL Error:GL_INVALID_FRAMEBUFFER_OPERATION \r\n");
            break;
        case GL_NO_ERROR:
            
            break;
        default:
            NSLog(@"Unknown GL Error %d \r\n",error);
            break;
    }
}


- (void) render
{
    if(overLay == NULL) {
        return;
    }
    if(program == 0) {
        [self initShader: overLay->format];
        [self initTextures];
        GetGLError();
    }
    if(/* DISABLES CODE */ (0)) {
        int ySize = overLay->width * overLay->height;
        FILE* file = fopen("/Users/chao/Desktop/ijk.yuv", "wb+");
        fwrite(overLay->data[0], 1, ySize, file);
        fwrite(overLay->data[1], 1, ySize / 4, file);
        fwrite(overLay->data[2], 1, ySize / 4, file);
        fclose(file);
    }
    //NSLog(@"render");
    [self checkGLError];
    glClearColor(0.f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(program);
    
    [renderLock lock];
    const int planes = overLay->planes;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    int widths[3] = {0};
    int heights[3] = {0};
    int formats[3] = {0};
    switch (overLay->format) {
        case FMT_YUV420P:
            widths[0] = overLay->linesize[0];
            widths[1] = widths[2] = (overLay->linesize[0] >> 1);
            heights[0] = overLay->height;
            heights[1] = heights[2] = (heights[0] >> 1);
            formats[0] = formats[1] = formats[2] = GL_LUMINANCE;
            break;
        case FMT_NV12:
            widths[0] = overLay->linesize[0];
            widths[1] = overLay->linesize[0] / 2;
            heights[0] = overLay->height;
            heights[1] = overLay->height / 2;
            formats[0] = GL_LUMINANCE;
            formats[1] = GL_LUMINANCE_ALPHA;
            break;
        default:
            break;
    }
    for (int i = 0; i < planes; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glUniform1i(uniformSamplers[i], i);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     formats[i],
                     (int) widths[i],
                     (int) heights[i],
                     0,
                     formats[i],
                     GL_UNSIGNED_BYTE,
                     overLay->data[i]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    [renderLock unlock];
    
    static const GLfloat squareVertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
        1.0f,  1.0f,
        1.0f, -1.0f,
    };
    
    static const GLfloat textureCoordinates[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
         1.0f, 0.0f,
        1.0f, 1.0f,
    };
    
    glEnableVertexAttribArray(aPostionLocation);
    glVertexAttribPointer(aPostionLocation, 2, GL_FLOAT, 0, 0, squareVertices);
    glEnableVertexAttribArray(aTextureCoordLocation);
    glVertexAttribPointer(aTextureCoordLocation, 2, GL_FLOAT, 0, 0, textureCoordinates);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

-(void)initTextures{
    glGenTextures(3, textures);
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
}

-(void)initShader:(RcColorFormat) format{
    ProgramTools* programTools = [[ProgramTools alloc]init];
    program = [programTools compileProgram:FLAT_VSH_STR frag:(format == FMT_YUV420P ? FLAT_FSH_YUV420P_STR : FLAT_FSH_NV12_STR)];
    glUseProgram(program);
    aPostionLocation = glGetAttribLocation(program, "a_Position");
    aTextureCoordLocation = glGetAttribLocation(program, "a_TexCoordinate");
    uniformSamplers[0] = glGetUniformLocation(program, "sample0");
    uniformSamplers[1] = glGetUniformLocation(program, "sample1");
    uniformSamplers[2] = glGetUniformLocation(program, "sample2");
    mMVPMatrixHandle = glGetUniformLocation(program, "u_MVPMatrix");
}

- (void)setImage:(RcFrame*)cacheOverlay {
    [renderLock lock];
    overLay = cacheOverlay;
    [renderLock unlock];
}

- (id) initWithDefaultFBO: (GLuint) defaultFBOName
{
	if((self = [super init]))
	{
		NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
		_defaultFBOName = defaultFBOName;
    }
	return self;
}


- (void) dealloc
{
	
}

@end

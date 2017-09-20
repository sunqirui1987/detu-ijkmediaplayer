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

typedef enum GlColorFormat {
    FMT_RGBA,
    FMT_YUV420P,
    FMT_NV12,
    FMT_VTB
}GlColorFormat;

static const GLfloat kColorConversion709[] = {
    1.164,  1.164,  1.164,
    0.0,   -0.213,  2.112,
    1.793, -0.533,  0.0,
};

#ifndef NULL
#define NULL 0
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

@interface OpenGLRenderer ()
{
    GLuint _defaultFBOName;
    GLuint _viewWidth;
    GLuint _viewHeight;
    GLuint textures[3];
    GLuint uniformSamplers[3];
    GLuint aPostionLocation;
    GLuint aTextureCoordLocation;
    GLuint aIsHardWare;
    GLuint colorConversion;
    GLuint mMVPMatrixHandle;
    GLuint uniColorFormat;
    GLuint program;
    SDL_VoutOverlay* overLay;
    int index;
}
@end

@implementation OpenGLRenderer


- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height
{
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
        overLay = (SDL_VoutOverlay*)calloc(1, sizeof(SDL_VoutOverlay));
        overLay->pitches = (Uint16 *)calloc(1, sizeof(Uint16) * 2);
        overLay->pixels = (Uint8 **)calloc(1, sizeof(Uint8 *) * 2);
        NSString* path = [[NSBundle mainBundle]  pathForResource:@"output" ofType:@"yuv"];
        FILE* file = fopen([path UTF8String], "r+");
        const int width = 2048;
        const int height = 1024;
        overLay->w = width;
        overLay->h = height;
        
        const int fakeWidth = 2048;
        const int fakeHeight = 1024;
        const int ySize = fakeWidth * fakeHeight;
        const int yuvSize = ySize * 3 / 2;
        unsigned char* yuvData = (unsigned char*)calloc(1, yuvSize);
        fread(yuvData, 1, yuvSize, file);
        fclose(file);
        
        overLay->pixels[0] = yuvData;
        overLay->pixels[1] = yuvData + ySize;
        overLay->planes = 2;
        overLay->pitches[0] = fakeWidth;
        overLay->pitches[1] = fakeWidth / 2;
        overLay->format = AV_PIX_FMT_NV12;
    }
    
    if(overLay == NULL) {
        return;
    }
    //NSLog(@"render");
    [self checkGLError];
    glClearColor(0.f, 1.0f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(program);
    const int planes = overLay->planes;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < planes; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glUniform1i(uniformSamplers[i], i);
    }
    const int widths[3] = {overLay->pitches[0], overLay->pitches[1], overLay->pitches[2]};
    const int heights[3] = { overLay->h, overLay->h / 2, overLay->h / 2 };
    for (int i = 0; i < planes; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_LUMINANCE,
                     (int) widths[i],
                     (int) heights[i],
                     0,
                     GL_LUMINANCE,
                     GL_UNSIGNED_BYTE,
                     overLay->pixels[i]);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    int overLayColorFormat = overLay->format;
    GlColorFormat glColorFormat = FMT_YUV420P;
    switch (overLayColorFormat) {
        case AV_PIX_FMT_YUV420P:
        glColorFormat = FMT_YUV420P;
        break;
        case AV_PIX_FMT_NV12:
        glColorFormat = FMT_NV12;
        break;
        case AV_PIX_FMT_RGBA:
        glColorFormat = FMT_RGBA;
        break;
        default:
        break;
    }
    glUniform1i(uniColorFormat, glColorFormat);
    
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
    glUniformMatrix3fv(colorConversion, 1, GL_FALSE, kColorConversion709);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

-(void)initTextures{
    glGenTextures(3, textures);
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }
}

-(void)initShader{
    ProgramTools* programTools = [[ProgramTools alloc]init];
    NSString *pathV = [[NSBundle mainBundle] pathForResource:@"sphere" ofType:@"vsh"];
    NSString* vStr = [[NSString alloc]initWithContentsOfFile:pathV];
    NSString *pathF = [[NSBundle mainBundle] pathForResource:@"sphere" ofType:@"fsh"];
    NSString* fStr = [[NSString alloc]initWithContentsOfFile:pathF];
    program = [programTools compileProgram:[vStr UTF8String] frag:[fStr UTF8String]];
    glUseProgram(program);
    aPostionLocation = glGetAttribLocation(program, "a_Position");
    aTextureCoordLocation = glGetAttribLocation(program, "a_TexCoordinate");
    uniformSamplers[0] = glGetUniformLocation(program, "SamplerY");
    uniformSamplers[1] = glGetUniformLocation(program, "SamplerU");
    uniformSamplers[2] = glGetUniformLocation(program, "SamplerV");
    aIsHardWare = glGetUniformLocation(program, "ishardware");
    colorConversion = glGetUniformLocation(program, "colorConversionMatrix");
    mMVPMatrixHandle = glGetUniformLocation(program, "u_MVPMatrix");
    uniColorFormat = glGetUniformLocation(program, "colorFormat");
}

- (void)setImage:(SDL_VoutOverlay*)cacheOverlay {
    overLay = cacheOverlay;
}

- (id) initWithDefaultFBO: (GLuint) defaultFBOName
{
	if((self = [super init]))
	{
		NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
		_defaultFBOName = defaultFBOName;
        [self initShader];
        [self initTextures];
		GetGLError();
	} 
	return self;
}


- (void) dealloc
{
	
}

@end

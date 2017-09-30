//
//  GlFlatRender.m
//  IJKMediaDemo
//
//  Created by chao on 2017/6/24.
//  Copyright © 2017年 detu. All rights reserved.
//

#import "GlFlatRender.h"
#import "ProgramTools.h"

#define FLOAT_SIZE_BYTES 4;
#define SHORT_SIZE_BYTES 2;
#define COORDS_PER_VERTEX 3;
#define TEXTURE_COORS_PER_VERTEX 2;

#define RE_GLES_STRINGIZE(x)   #x


static const struct
{
    float x, y;
    float r, g, b;
} vertices[4] =
{
    { -1.f, 1.f, 0.f, 0.f},
    {  1.f, 1.f, 1.f, 0.f },
    { -1.f, -1.f, 0.f, 1.f },
    { 1.f, -1.f, 1.f, 1.f },
};

static const GLfloat kColorConversion709[] = {
    1.164,  1.164,  1.164,
    0.0,   -0.213,  2.112,
    1.793, -0.533,  0.0,
};

static const char FLAT_VSH_STR[] = RE_GLES_STRINGIZE(
                                                     uniform mat4 u_MVPMatrix;
                                                     attribute vec4 a_Position;
                                                     attribute  vec2 a_TexCoordinate;
                                                     varying vec2 v_TexCoordinate;
                                                     void main()
{
    v_TexCoordinate = a_TexCoordinate;
    gl_Position = u_MVPMatrix * a_Position;
    
}
                                                     );

static const char FLAT_FSH_STR[] = RE_GLES_STRINGIZE(
                                                     varying vec2 v_TexCoordinate;
                                                     uniform  sampler2D SamplerY;
                                                     uniform  sampler2D SamplerU;
                                                     uniform  sampler2D SamplerV;
                                                     uniform mat3 colorConversionMatrix;
                                                     void main(){
                                                         vec3 yuv;
                                                         vec3 rgb;
                                                         yuv.x = (texture2D(SamplerY, v_TexCoordinate).r - (16.0 / 255.0));
                                                         yuv.y = (texture2D(SamplerU, v_TexCoordinate).r - 0.5);
                                                         yuv.z = (texture2D(SamplerV, v_TexCoordinate).r - 0.5);
                                                         rgb = colorConversionMatrix * yuv;
                                                         gl_FragColor = vec4(rgb, 1);
                                                     }
                                                     );

@interface GlFlatRender(){
    GLuint program;
    GLuint aPostionLocation;
    GLuint aTextureCoordLocation;
    GLuint aIsHardWare;
    GLuint textures[3];
    GLint uniformSamplers[3];
    GLuint colorConversion;
    GLuint mMVPMatrixHandle;
    GLuint vertex_buffer;
}
@end

@implementation GlFlatRender


-(void)initCoords{
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(aPostionLocation);
    glVertexAttribPointer(aPostionLocation, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 4, (void*) 0);
    glEnableVertexAttribArray(aTextureCoordLocation);
    glVertexAttribPointer(aTextureCoordLocation, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 4, (void*) (sizeof(float) * 2));
}

-(void)initTextures{
    glGenTextures(3, textures);
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
}

- (void) initWithDefaultFBO: (GLuint) defaultFBOName {
    
    [self initShader];
    [self initTextures];
    [self initCoords];
}
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height {
    glViewport(0, 0, width, height);
     glClearColor(0.f, 1.f, 1.f, 1.f);
}
- (void) render {
    //NSLog(@"render");
   
    if(overLay == NULL) {
        return;
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glUseProgram(program);
    const int widths[3] = {overLay->pitches[0], overLay->pitches[1], overLay->pitches[2]};
    const int heights[3] = { overLay->h, overLay->h / 2, overLay->h / 2 };
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < 3; ++i) {
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
    for (int i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glUniform1i(uniformSamplers[i], i);
    }
    glUniform1f(aIsHardWare, 0);
    glUniformMatrix3fv(colorConversion, 1, GL_FALSE, kColorConversion709);
    GLfloat matrix[16] = {1.0f,0.0f,0.0f,0.0f,    0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f,  0.0f,0.0f,0.0f,1.0f};
    //glUniformMatrix4fv(mMVPMatrixHandle, 1, false, matrix);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

}
- (void) dealloc {
}
@end

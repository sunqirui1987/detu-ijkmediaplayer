//
//  ProgramTools.m
//  IJKMediaDemo
//
//  Created by chao on 2017/6/20.
//  Copyright © 2017年 detu. All rights reserved.
//

#import "ProgramTools.h"
#import "glUtil.h"

@implementation ProgramTools

-(GLuint)compileProgram:(NSString*)sharder {
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program
    GLuint programHandler = glCreateProgram();
    
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
    
    //*shader = glCreateShaderProgramv(type, 1, source);
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



static bool re_gles2_compileShader(GLuint *shader,GLenum type,const char *str)
{
    GLint status;
    const GLchar *source;
    
    source = str;
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
    
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        
        NSLog(@"compileShader:\n%s\n \n", log);
        
        free(log);
    }
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*shader);
        return false;
    }
    
    return true;
}

static bool re_gles2_linkProgram(GLuint prog)
{
    GLint status;
    
    glLinkProgram(prog);
    
    
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"linkProgram:\n%s", log);
        
        free(log);
    }
    
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return false;
    
    return true;
}

static bool re_gles2_validateProgram(GLuint prog)
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
        return false;
    
    return true;
}

static GLuint re_gles2_loadShaders(const char* vsh_str ,const char* fsh_str)
{
    float  glLanguageVersion;
    
#if TARGET_IOS
    sscanf((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "OpenGL ES GLSL ES %f", &glLanguageVersion);
#else
    sscanf((char *)glGetString(GL_SHADING_LANGUAGE_VERSION), "%f", &glLanguageVersion);
#endif
    
    // GL_SHADING_LANGUAGE_VERSION returns the version standard version form
    //  with decimals, but the GLSL version preprocessor directive simply
    //  uses integers (thus 1.10 should 110 and 1.40 should be 140, etc.)
    //  We multiply the floating point number by 100 to get a proper
    //  number for the GLSL preprocessor directive
    GLuint version = 100 * glLanguageVersion;
    version = 330;
    
    
    char vshStr[20000];
    char fshStr[20000];
    
    //sprintf(vshStr, "#version %d\n%s", version, vsh_str);
    //sprintf(fshStr, "#version %d\n%s", version, fsh_str);
    
    sprintf(vshStr, "%s", vsh_str);
    sprintf(fshStr, "%s", fsh_str);

    GLuint vertShader, fragShader;
    
    // Create shader program
    GLuint mProgramHandle = glCreateProgram();
    if (!re_gles2_compileShader(&vertShader,GL_VERTEX_SHADER,vshStr))
    {
        NSLog(@"Failed to compile vertex shader");
        return 0;
    }
    
    
    if (!re_gles2_compileShader(&fragShader,GL_FRAGMENT_SHADER,fshStr))
    {
        NSLog(@"Failed to compile vertex shader");
        return 0;
    }
    
    
    // Attach vertex shader to program
    glAttachShader(mProgramHandle, vertShader);
    
    // Attach fragment shader to program
    glAttachShader(mProgramHandle, fragShader);
    
    
    // Link program
    if (!re_gles2_linkProgram(mProgramHandle))
    {
        NSLog(@"Failed to link program: %d", mProgramHandle);
        
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
        if (mProgramHandle)
        {
            glDeleteProgram(mProgramHandle);
            mProgramHandle = 0;
        }
        
        return 0;
    }
    // Release vertex and fragment shaders
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    return mProgramHandle;
}

-(GLuint)compileProgram:(const char *)vertextStr frag:(const char *)fragStr {
    return re_gles2_loadShaders(vertextStr, fragStr);
}

@end

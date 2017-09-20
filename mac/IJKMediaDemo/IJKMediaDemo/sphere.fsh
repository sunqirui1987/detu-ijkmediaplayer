#if __VERSION__ >= 140
out vec4     fragColor;
in vec2 v_TexCoordinate;
#else
varying vec2 v_TexCoordinate;
#endif
uniform  sampler2D SamplerY;
uniform  sampler2D SamplerU;
uniform  sampler2D SamplerV;
uniform mat3 colorConversionMatrix;
void main(){
    vec3 yuv;
    vec3 rgb;
   #if __VERSION__ >= 140
    yuv.x = (texture(SamplerY, v_TexCoordinate).r - (16.0 / 255.0));
    yuv.y = (texture(SamplerU, v_TexCoordinate).r - 0.5);
    yuv.z = (texture(SamplerV, v_TexCoordinate).r - 0.5);
    rgb = colorConversionMatrix * yuv;
    fragColor = vec4(rgb, 1);
    
    #else
    yuv.x = (texture2D(SamplerY, v_TexCoordinate).r - (16.0 / 255.0));
    yuv.y = (texture2D(SamplerU, v_TexCoordinate).r - 0.5);
    yuv.z = (texture2D(SamplerV, v_TexCoordinate).r - 0.5);
    rgb = colorConversionMatrix * yuv;
    gl_FragColor = vec4(rgb, 1);

    #endif

}


const int FMT_RGBA = 0;
const int FMT_YUV420P = 1;
const int FMT_NV12 = 2;
const int FMT_VTB = 3;
varying vec2 v_TexCoordinate;
uniform  sampler2D SamplerY;
uniform  sampler2D SamplerU;
uniform  sampler2D SamplerV;
uniform mat3 colorConversionMatrix;
uniform int colorFormat;

const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);
const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);
void main(){
    vec3 yuv;
    vec3 rgb;
    if(colorFormat == FMT_RGBA) {
        
    } else if(colorFormat == FMT_YUV420P) {
        yuv.x = (texture2D(SamplerY, v_TexCoordinate).r - (16.0 / 255.0));
        yuv.y = (texture2D(SamplerU, v_TexCoordinate).r - 0.5);
        yuv.z = (texture2D(SamplerV, v_TexCoordinate).r - 0.5);
    } else if(colorFormat == FMT_NV12) {
        //yuv.x = (texture2D(SamplerY, v_TexCoordinate).r - (16.0 / 255.0));
        //yuv.yz = (texture2D(SamplerU, v_TexCoordinate).rg - vec2(0.5, 0.5));
        yuv.x = texture2D(SamplerY, v_TexCoordinate).r;
        yuv.yz = texture2D(SamplerU, v_TexCoordinate).rg;
        yuv += offset;
        rgb.x = dot(yuv, Rcoeff);
        rgb.y = dot(yuv, Gcoeff);
        rgb.z = dot(yuv, Bcoeff);
    }
    //rgb = colorConversionMatrix * yuv;
    gl_FragColor = vec4(rgb, 1);
}


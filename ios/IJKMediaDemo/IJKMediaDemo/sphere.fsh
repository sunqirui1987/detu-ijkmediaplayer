precision mediump float;
uniform sampler2D u_Texture;


//uniform sampler2D SamplerUV;

varying vec2 v_TexCoordinate;
uniform bool forward;
uniform bool ishardware;


uniform lowp sampler2D SamplerY;
uniform lowp sampler2D SamplerU;
uniform lowp sampler2D SamplerV;



uniform   lowp  sampler2D us2_SamplerX;
uniform   lowp  sampler2D us2_SamplerY;

uniform mat3 colorConversionMatrix;
void main(){
    mediump vec3 yuv;
    lowp vec3 rgb;
    
    if(ishardware){
        yuv.x  = (texture2D(us2_SamplerX,  v_TexCoordinate).r  - (16.0 / 255.0));
        yuv.yz = (texture2D(us2_SamplerY,  v_TexCoordinate).rg - vec2(0.5, 0.5));
        
    }else{
        yuv.x = (texture2D(SamplerY, v_TexCoordinate).r - (16.0/255.0));
        yuv.y = (texture2D(SamplerU, v_TexCoordinate).r - 0.5);
        yuv.z = (texture2D(SamplerV, v_TexCoordinate).r - 0.5);
    }
    
    rgb = colorConversionMatrix * yuv;
    gl_FragColor = vec4(rgb, 1);
    
}

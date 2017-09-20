#if __VERSION__ >= 140
in vec4 a_Position;
in vec2 a_TexCoordinate;
out vec2 v_TexCoordinate;
#else
attribute vec4 a_Position;
attribute vec2 a_TexCoordinate;
varying vec2 v_TexCoordinate;
#endif
void main()
{
    v_TexCoordinate = a_TexCoordinate;
    gl_Position = a_Position;
}

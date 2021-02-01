precision lowp float;

uniform lowp vec4 u_color;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = u_color;
}

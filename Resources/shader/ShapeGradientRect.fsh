precision float;

uniform vec4 u_color;
varying vec2 v_texCoord;

#define M_PI        3.14159265358979323846264338327950288

void main()
{
    lowp float a = cos(0.5 * M_PI * v_texCoord.y);
    gl_FragColor = u_color * a;
}

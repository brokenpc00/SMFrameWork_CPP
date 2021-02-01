uniform vec4 u_color;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor =  texture2D(CC_Texture0, v_texCoord) * u_color;
}

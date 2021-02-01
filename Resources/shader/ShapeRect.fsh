uniform vec2 u_dimension;
uniform float u_lineWidth;
uniform vec4 u_color;

varying vec2 v_texCoord;

void main()
{
    vec2 s = abs(u_dimension * (v_texCoord - 0.5));
    vec2 p = s - u_dimension * .5 + u_lineWidth;

    float c = 0.0;
    if (p.x > 0.0 || p.y >= 0.0) c = 1.0;
    
    gl_FragColor = u_color * c;
}

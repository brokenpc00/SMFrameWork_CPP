uniform vec4 u_color;
uniform vec2 u_center;
uniform float u_aspect;
uniform float u_strength;

varying vec2 v_texCoord;

void main()
{
    vec2 p = (v_texCoord - u_center) * u_aspect;
    
    float rf = sqrt(dot(p, p)) * u_strength;
    float rf2_1 = rf * rf + 1.0;
    float a = 1.0 - 1.0 / (rf2_1 * rf2_1);
    
    gl_FragColor = vec4(u_color.rgb, u_color.a * a);
}

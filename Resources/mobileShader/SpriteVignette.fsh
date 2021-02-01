precision mediump float;

uniform vec2 u_center;
uniform float u_aspect;
uniform float u_strength;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    vec2 p = (v_texCoord - u_center) * u_aspect;
    
    float rf = sqrt(dot(p, p)) * u_strength;
    float rf2_1 = rf * rf + 1.0;
    float c = 1.0 / (rf2_1 * rf2_1);
    
    gl_FragColor = c * v_fragmentColor * texture2D(CC_Texture0, v_texCoord);
}

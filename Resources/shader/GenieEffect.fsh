uniform vec2 u_anchor;
uniform float u_progress;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

const vec4 ZERO = vec4(0.0, 0.0, 0.0, 0.0);

void main()
{
    vec2 coord = v_texCoord + normalize(v_texCoord - u_anchor) * u_progress;
    
    if (coord.x >= 0.0 && coord.x <= 1.0 && coord.y >= 0.0 && coord.y <= 1.0) {
        gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, coord);
    } else {
        gl_FragColor = ZERO;
    }
}

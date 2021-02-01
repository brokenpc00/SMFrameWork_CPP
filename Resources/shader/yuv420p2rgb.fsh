uniform sampler2D u_textureU;
uniform sampler2D u_textureV;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    float y = 1.1643 * (texture2D(CC_Texture0, v_texCoord).x - 0.0625);
    float u = texture2D(u_textureU, v_texCoord).y - 0.5;
    float v = texture2D(u_textureV, v_texCoord).z - 0.5;
    float r = y + 1.5958 * v;
    float g = y - 0.39173 * u - 0.81290 * v;
    float b = y + 2.017 * u;
    
    gl_FragColor = v_fragmentColor * vec4(r, g, b, 1.0);
}



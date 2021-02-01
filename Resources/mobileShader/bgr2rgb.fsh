precision mediump float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    vec4 bgra = texture2D(CC_Texture0, v_texCoord);

    gl_FragColor = v_fragmentColor * vec4(bgra.b, bgra.g, bgra.r, 1.0);
}



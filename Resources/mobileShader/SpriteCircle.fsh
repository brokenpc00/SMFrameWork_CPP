precision mediump float;

uniform float u_radius;
uniform float u_aaWidth;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

void main()
{
    float dist = 1.0 - 2.0 * distance( v_texCoord, vec2( 0.5, 0.5 ) );
    float c = clamp( dist/(u_aaWidth/u_radius), 0.0, 1.0 );
    
    gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord) * c;
      
}




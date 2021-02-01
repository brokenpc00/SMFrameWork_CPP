precision mediump float; 

uniform float u_radius;
uniform float u_aaWidth;
uniform vec2 u_anchor;
uniform lowp vec4 u_color;

varying vec2 v_texCoord;

void main()
{
	float dist = 1.0 - 2.0 * distance( v_texCoord, u_anchor );
	float c = clamp( dist/(u_aaWidth/u_radius), 0.0, 1.0 );
	
    gl_FragColor = u_color * c;
} 



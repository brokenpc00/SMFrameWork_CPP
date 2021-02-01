uniform float u_radius;
uniform float u_aaWidth;
uniform float u_lineWidth;
uniform vec4 u_color;

varying vec2 v_texCoord;

void main()
{
    float p = 0.5 * u_lineWidth / u_radius;
    float ceneter = 1.0 - p;
    float dist = 1.0 - abs( ceneter - 2.0 * distance( v_texCoord, vec2( 0.5, 0.5 ) ) ) / p;
	float c = clamp( dist / (u_aaWidth / (0.5 * u_lineWidth)), 0.0, 1.0);
	
    gl_FragColor = u_color * c;
} 



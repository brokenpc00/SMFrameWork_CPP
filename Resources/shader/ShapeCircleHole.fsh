uniform vec2 u_center;
uniform vec2 u_dimension;
uniform float u_radius;
uniform float u_aaWidth;
uniform vec4 u_color;

varying vec2 v_texCoord;

void main()
{
    float dist = distance(v_texCoord * u_dimension, u_center) - u_radius;
	float c = clamp( dist/u_aaWidth, 0.0, 1.0 );
	
    gl_FragColor = u_color * c;
} 



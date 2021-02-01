uniform sampler2D u_texture;

uniform vec2 u_dimension;
uniform float u_radius;
uniform float u_aaWidth;

uniform vec4 u_color;
varying vec2 v_texCoord;

void main()
{
    vec2 p = v_texCoord * u_dimension;
    float dist = 1.0 - distance(p, 0.5 * u_dimension) / u_radius;
    float c = clamp( dist/(u_aaWidth/u_radius), 0.0, 1.0 );
    
    gl_FragColor = u_color * texture2D(u_texture, v_texCoord) * c;
}




uniform float u_radius;
uniform float u_aaWidth;
uniform float u_lineWidth;
uniform float u_theta;
uniform vec4 u_color;

varying vec2 v_texCoord;

#define M_PI        3.14159265358979323846264338327950288

float calcurateCapColor(vec2 origin, vec2 pos, float radius) {
    float dist = 1.0 - 2.0 * distance( pos, origin ) / radius;
    
    return clamp( dist / (u_aaWidth / (0.5 * u_lineWidth)), 0.0, 1.0 );
}

void main()
{
    vec2 pos = v_texCoord - vec2(0.5, 0.5);
    float angle = M_PI + atan(pos.y, pos.x);
    
    float lineRadius = 0.5 * u_lineWidth / u_radius;
    float color;

    if (angle < u_theta) {
        // draw ring
        float origin = 1.0 - lineRadius;
        float dist = 1.0 - abs(origin - 2.0 * length(pos)) / lineRadius;
        
        color = clamp( dist / (u_aaWidth / (0.5 * u_lineWidth)), 0.0, 1.0);
        
    } else {
        // draw cap
        float radius = 0.5 - 0.5 * lineRadius;
        vec2 origin1 = radius * vec2(-1.0, 0.0);
        vec2 origin2 = radius * vec2(-cos(u_theta), -sin(u_theta));
        
        color = calcurateCapColor(origin1, pos, lineRadius) + calcurateCapColor(origin2, pos, lineRadius);
        
    }
    
    gl_FragColor = u_color * color;
}


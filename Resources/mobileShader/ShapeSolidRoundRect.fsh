precision highp float;

uniform vec2 u_dimension;
uniform float u_radius;
uniform float u_aaWidth;
uniform lowp vec4 u_color;

varying highp vec2 v_texCoord;

// ver 7.0
void main()
{
    highp vec2 s = abs(u_dimension * (v_texCoord - 0.5));
    highp vec2 p = s - u_dimension * .5 + u_radius;
    float edge = u_radius - u_aaWidth;
    float c = 1.0;
    
    if (p.x > 0.0)
    {
        if (p.y > 0.0)
        {   // 코너
            c = smoothstep(u_radius, edge, length(p));
        }
        else if (p.x > edge)
        {   // 우측 AA
            c = smoothstep(u_radius, edge, p.x);
        }
    }
    else if (p.y > edge)
    {   // 상단 AA
        c = smoothstep(u_radius, edge, p.y);
    }
    
    gl_FragColor = u_color * c;
}

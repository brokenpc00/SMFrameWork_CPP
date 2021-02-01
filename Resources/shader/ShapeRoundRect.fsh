uniform vec2 u_dimension;
uniform float u_radius;
uniform float u_lineWidth;
uniform float u_aaWidth;
uniform vec4 u_color;

varying vec2 v_texCoord;

void main()
{
    vec2 s = abs(u_dimension * (v_texCoord - 0.5));
    vec2 p = s - u_dimension * .5 + u_radius;
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
    
    float scale = 1.0 - u_lineWidth / u_radius;
    float innerRadius = u_radius * scale;
    edge = innerRadius - u_aaWidth;
    s = s * scale;
    
    float c2 = 1.0;
    if (p.x > 0.0)
    {
        if (p.y > 0.0)
        {   // 코너
            c2 = smoothstep(innerRadius, edge, length(p));
        }
        else if (p.x > edge)
        {   // 우측 AA
            c2 = smoothstep(innerRadius, edge, p.x);
        }
    }
    else if (p.y > edge)
    {   // 상단 AA
        c2 = smoothstep(innerRadius, edge, p.y);
    }
    
    gl_FragColor = u_color * c * (1.0 - c2);
}


varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform vec2 resolution;
uniform float blurRadius;
uniform float sampleNum;

vec4 blur(vec2);

void main(void)
{
    vec4 col = blur(v_texCoord); //* v_fragmentColor.rgb;
    gl_FragColor = vec4(col) * v_fragmentColor;
}

vec4 blur(vec2 p)
{
    if (blurRadius > 0.0 && sampleNum > 1.0)
    {
        vec4 col = vec4(0);
        vec2 unit = 1.0 / resolution.xy;
        
        float r = blurRadius;
        float sampleStep = r / sampleNum;
        
        float count = 0.0;
        
        for(float x = -r; x < r; x += sampleStep)
        {
            for(float y = -r; y < r; y += sampleStep)
            {
                float weight = (r - abs(x)) * (r - abs(y));
                col += texture2D(CC_Texture0, p + vec2(x * unit.x, y * unit.y)) * weight;
                count += weight;
            }
        }
        
        return col / count;
    }
    
    return texture2D(CC_Texture0, p);
}

//varying vec4 v_fragmentColor;
//varying vec2 v_texCoord;
//
//#define GAUSSIAN_SAMPLES 9
//varying vec2 v_blurCoord[GAUSSIAN_SAMPLES];
//
//void main()
//{
//    float a = 0.0;
//    
//    /*
//     a += texture2D(CC_Texture0, v_blurCoord[0]).a * 0.204164;
//     a += texture2D(CC_Texture0, v_blurCoord[1]).a * 0.304005;
//     a += texture2D(CC_Texture0, v_blurCoord[2]).a * 0.304005;
//     a += texture2D(CC_Texture0, v_blurCoord[3]).a * 0.093913;
//     a += texture2D(CC_Texture0, v_blurCoord[4]).a * 0.093913;
//     */
//    
//    a += texture2D(CC_Texture0, v_blurCoord[0]).a * 0.05;
//    a += texture2D(CC_Texture0, v_blurCoord[1]).a * 0.09;
//    a += texture2D(CC_Texture0, v_blurCoord[2]).a * 0.12;
//    a += texture2D(CC_Texture0, v_blurCoord[3]).a * 0.15;
//    a += texture2D(CC_Texture0, v_blurCoord[4]).a * 0.18;
//    a += texture2D(CC_Texture0, v_blurCoord[5]).a * 0.15;
//    a += texture2D(CC_Texture0, v_blurCoord[6]).a * 0.12;
//    a += texture2D(CC_Texture0, v_blurCoord[7]).a * 0.09;
//    a += texture2D(CC_Texture0, v_blurCoord[8]).a * 0.05;
//    
//    
//    
//    //    gl_FragColor = texture2D(CC_Texture0, v_texCoord);//vec4(0.0, 0.0, 0.0, a);
//    gl_FragColor = v_fragmentColor * vec4(0.0, 0.0, 0.0, a);
//}

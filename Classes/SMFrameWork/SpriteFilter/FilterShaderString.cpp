//
//  FilterShaderString.cpp
//  iPet
//
//  Created by KimSteve on 2017. 5. 25..
//  Copyright © 2017년 KimSteve. All rights reserved.
//

#include "FilterShaderString.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Base Vertext Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_VS_BASE = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

// 넘겨줄거
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;

void main()
{
    gl_Position = CC_PMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoord = a_texCoord;
}
)";

// base fragment
//    gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER LordKelvin Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_KEVLVIN = R"(

precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 lookup;
    lookup.y = .5;
    
    lookup.x = texel.r;
    texel.r = texture2D(filterTexture, lookup).r;
    
    lookup.x = texel.g;
    texel.g = texture2D(filterTexture, lookup).g;
    
    lookup.x = texel.b;
    texel.b = texture2D(filterTexture, lookup).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}
)";


///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER 1977 Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_1977 = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
//uniform sampler2D filterTexture2;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    texel = vec3(
                 texture2D(filterTexture1, vec2(texel.r, .16666)).r,
                 texture2D(filterTexture1, vec2(texel.g, .5)).g,
                 texture2D(filterTexture1, vec2(texel.b, .83333)).b);
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}

)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Amaro Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_AMARO = R"(

precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
uniform sampler2D filterTexture2;
uniform sampler2D filterTexture3;

void main()
{
    
    vec4 texel = texture2D(CC_Texture0, v_texCoord);
    vec3 bbTexel = texture2D(filterTexture1, v_texCoord).rgb;
    
    texel.r = texture2D(filterTexture2, vec2(bbTexel.r, texel.r)).r;
    texel.g = texture2D(filterTexture2, vec2(bbTexel.g, texel.g)).g;
    texel.b = texture2D(filterTexture2, vec2(bbTexel.b, texel.b)).b;
    
    vec4 mapped;
    mapped.r = texture2D(filterTexture3, vec2(texel.r, .16666)).r;
    mapped.g = texture2D(filterTexture3, vec2(texel.g, .5)).g;
    mapped.b = texture2D(filterTexture3, vec2(texel.b, .83333)).b;
    mapped.a = 1.0;
    
    gl_FragColor = v_fragmentColor * mapped;
}

)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Brannan Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_BRANNAN = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //process
uniform sampler2D filterTexture2;  //blowout
//uniform sampler2D filterTexture3;  //contrast
uniform sampler2D filterTexture4;  //luma
uniform sampler2D filterTexture5;  //screen

mat3 saturateMatrix = mat3(
                           1.105150,
                           -0.044850,
                           -0.046000,
                           -0.088050,
                           1.061950,
                           -0.089200,
                           -0.017100,
                           -0.017100,
                           1.132900);

vec3 luma = vec3(.3, .59, .11);

void main()
{
    

//
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 lookup;
    lookup.y = 0.5;
    lookup.x = texel.r;
    texel.r = texture2D(filterTexture1, lookup).r;
    lookup.x = texel.g;
    texel.g = texture2D(filterTexture1, lookup).g;
    lookup.x = texel.b;
    texel.b = texture2D(filterTexture1, lookup).b;
    
    texel = saturateMatrix * texel;

    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    vec3 sampled;
    lookup.y = 0.5;
    lookup.x = texel.r;
    sampled.r = texture2D(filterTexture2, lookup).r;
    lookup.x = texel.g;
    sampled.g = texture2D(filterTexture2, lookup).g;
    lookup.x = texel.b;
    sampled.b = texture2D(filterTexture2, lookup).b;
    float value = smoothstep(0.0, 1.0, d);
    texel = mix(sampled, texel, value);

//    lookup.x = texel.r;
//    texel.r = texture2D(filterTexture3, lookup).r;
//    lookup.x = texel.g;
//    texel.g = texture2D(filterTexture3, lookup).g;
//    lookup.x = texel.b;
//    texel.b = texture2D(filterTexture3, lookup).b;
    
    lookup.x = dot(texel, luma);
    texel = mix(texture2D(filterTexture4, lookup).rgb, texel, .5);
    
    lookup.x = texel.r;
    texel.r = texture2D(filterTexture5, lookup).r;
    lookup.x = texel.g;
    texel.g = texture2D(filterTexture5, lookup).g;
    lookup.x = texel.b;
    texel.b = texture2D(filterTexture5, lookup).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}

)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Early Bird Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_EARLYBIRD = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //earlyBirdCurves
uniform sampler2D filterTexture2;  //earlyBirdOverlay
uniform sampler2D filterTexture3;  //vig
//uniform sampler2D filterTexture4;  //earlyBirdBlowout
uniform sampler2D filterTexture5;  //earlyBirdMap

const mat3 saturate = mat3(
                           1.210300,
                           -0.089700,
                           -0.091000,
                           -0.176100,
                           1.123900,
                           -0.177400,
                           -0.034200,
                           -0.034200,
                           1.265800);
const vec3 rgbPrime = vec3(0.25098, 0.14640522, 0.0);
const vec3 desaturate = vec3(.3, .59, .11);

void main()
{
    //
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 lookup;
    lookup.y = 0.5;
    
    lookup.x = texel.r;
    texel.r = texture2D(filterTexture1, lookup).r;
    
    lookup.x = texel.g;
    texel.g = texture2D(filterTexture1, lookup).g;
    
    lookup.x = texel.b;
    texel.b = texture2D(filterTexture1, lookup).b;
    
    float desaturatedColor;
    vec3 result;
    desaturatedColor = dot(desaturate, texel);
    
    
    lookup.x = desaturatedColor;
    result.r = texture2D(filterTexture2, lookup).r;
    lookup.x = desaturatedColor;
    result.g = texture2D(filterTexture2, lookup).g;
    lookup.x = desaturatedColor;
    result.b = texture2D(filterTexture2, lookup).b;
    
    texel = saturate * mix(texel, result, .5);
    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    
    vec3 sampled;
    lookup.y = .5;
    
    /*
     lookup.x = texel.r;
     sampled.r = texture2D(filterTexture3, lookup).r;
     
     lookup.x = texel.g;
     sampled.g = texture2D(filterTexture3, lookup).g;
     
     lookup.x = texel.b;
     sampled.b = texture2D(filterTexture3, lookup).b;
     
     float value = smoothstep(0.0, 1.25, pow(d, 1.35)/1.65);
     texel = mix(texel, sampled, value);
     */
    
    //---
    
    lookup = vec2(d, texel.r);
    texel.r = texture2D(filterTexture3, lookup).r;
    lookup.y = texel.g;
    texel.g = texture2D(filterTexture3, lookup).g;
    lookup.y = texel.b;
    texel.b	= texture2D(filterTexture3, lookup).b;
    float value = smoothstep(0.0, 1.25, pow(d, 1.35)/1.65);
    
    //---
    
    lookup.x = texel.r;
    sampled.r = texture2D(filterTexture5, lookup).r;
    lookup.x = texel.g;
    sampled.g = texture2D(filterTexture5, lookup).g;
    lookup.x = texel.b;
    sampled.b = texture2D(filterTexture5, lookup).b;
    texel = mix(sampled, texel, value);
    
    
    lookup.x = texel.r;
    texel.r = texture2D(filterTexture5, lookup).r;
    lookup.x = texel.g;
    texel.g = texture2D(filterTexture5, lookup).g;
    lookup.x = texel.b;
    texel.b = texture2D(filterTexture5, lookup).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}

)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Hefe Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_HEFE = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //edgeBurn
uniform sampler2D filterTexture2;  //hefeMap
uniform sampler2D filterTexture3;  //hefeGradientMap
uniform sampler2D filterTexture4;  //hefeSoftLight
uniform sampler2D filterTexture5;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    vec3 edge = texture2D(filterTexture1, v_texCoord).rgb;
    texel = texel * edge;
    
    texel = vec3(
                 texture2D(filterTexture2, vec2(texel.r, .16666)).r,
                 texture2D(filterTexture2, vec2(texel.g, .5)).g,
                 texture2D(filterTexture2, vec2(texel.b, .83333)).b);
    
    vec3 luma = vec3(.30, .59, .11);
    vec3 gradSample = texture2D(filterTexture3, vec2(dot(luma, texel), .5)).rgb;
    vec3 final = vec3(
                      texture2D(filterTexture4, vec2(gradSample.r, texel.r)).r,
                      texture2D(filterTexture4, vec2(gradSample.g, texel.g)).g,
                      texture2D(filterTexture4, vec2(gradSample.b, texel.b)).b
                      );
    
    vec3 metal = texture2D(filterTexture5, v_texCoord).rgb;
    vec3 metaled = vec3(
                        texture2D(filterTexture4, vec2(metal.r, texel.r)).r,
                        texture2D(filterTexture4, vec2(metal.g, texel.g)).g,
                        texture2D(filterTexture4, vec2(metal.b, texel.b)).b
                        );
    
    gl_FragColor = v_fragmentColor * vec4(metaled, 1.0);
}
)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Hudson Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_HUDSON = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //blowout;
uniform sampler2D filterTexture2;  //overlay;
uniform sampler2D filterTexture3;  //map

void main()
{
    
    vec4 texel = texture2D(CC_Texture0, v_texCoord);
    
    vec3 bbTexel = texture2D(filterTexture1, v_texCoord).rgb;
    
    texel.r = texture2D(filterTexture2, vec2(bbTexel.r, texel.r)).r;
    texel.g = texture2D(filterTexture2, vec2(bbTexel.g, texel.g)).g;
    texel.b = texture2D(filterTexture2, vec2(bbTexel.b, texel.b)).b;
    
    vec4 mapped;
    mapped.r = texture2D(filterTexture3, vec2(texel.r, .16666)).r;
    mapped.g = texture2D(filterTexture3, vec2(texel.g, .5)).g;
    mapped.b = texture2D(filterTexture3, vec2(texel.b, .83333)).b;
    mapped.a = 1.0;
    gl_FragColor = v_fragmentColor * mapped;
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Inkwell Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_INKWELL = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    texel = vec3(dot(vec3(0.3, 0.6, 0.1), texel));
    texel = vec3(texture2D(filterTexture, vec2(texel.r, .16666)).r);
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}
)";



///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Lomofi Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_LOMOFI = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
uniform sampler2D filterTexture2;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 red = vec2(texel.r, 0.16666);
    vec2 green = vec2(texel.g, 0.5);
    vec2 blue = vec2(texel.b, 0.83333);
    
    texel.rgb = vec3(
                     texture2D(filterTexture1, red).r,
                     texture2D(filterTexture1, green).g,
                     texture2D(filterTexture1, blue).b);
    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    vec2 lookup = vec2(d, texel.r);
    texel.r = texture2D(filterTexture2, lookup).r;
    lookup.y = texel.g;
    texel.g = texture2D(filterTexture2, lookup).g;
    lookup.y = texel.b;
    texel.b	= texture2D(filterTexture2, lookup).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel,1.0);
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Nashville Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_NASHVILLE = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    texel = vec3(
                 texture2D(filterTexture, vec2(texel.r, .16666)).r,
                 texture2D(filterTexture, vec2(texel.g, .5)).g,
                 texture2D(filterTexture, vec2(texel.b, .83333)).b);
    gl_FragColor = vec4(texel, 1.0);
}
)";





///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Rise Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_RISE = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //blowout;
uniform sampler2D filterTexture2;  //overlay;
uniform sampler2D filterTexture3;  //map

void main()
{
    vec4 texel = texture2D(CC_Texture0, v_texCoord);
    vec3 bbTexel = texture2D(filterTexture1, v_texCoord).rgb;
    
    texel.r = texture2D(filterTexture2, vec2(bbTexel.r, texel.r)).r;
    texel.g = texture2D(filterTexture2, vec2(bbTexel.g, texel.g)).g;
    texel.b = texture2D(filterTexture2, vec2(bbTexel.b, texel.b)).b;
    
    vec4 mapped;
    mapped.r = texture2D(filterTexture3, vec2(texel.r, .16666)).r;
    mapped.g = texture2D(filterTexture3, vec2(texel.g, .5)).g;
    mapped.b = texture2D(filterTexture3, vec2(texel.b, .83333)).b;
    mapped.a = 1.0;
    
    gl_FragColor = v_fragmentColor * mapped;
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Sierra Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_SIERRA = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //blowout;
uniform sampler2D filterTexture2;  //overlay;
uniform sampler2D filterTexture3;  //map

void main()
{
    vec4 texel = texture2D(CC_Texture0, v_texCoord);
    vec3 bbTexel = texture2D(filterTexture1, v_texCoord).rgb;
    
    texel.r = texture2D(filterTexture2, vec2(bbTexel.r, texel.r)).r;
    texel.g = texture2D(filterTexture2, vec2(bbTexel.g, texel.g)).g;
    texel.b = texture2D(filterTexture2, vec2(bbTexel.b, texel.b)).b;
    
    vec4 mapped;
    mapped.r = texture2D(filterTexture3, vec2(texel.r, .16666)).r;
    mapped.g = texture2D(filterTexture3, vec2(texel.g, .5)).g;
    mapped.b = texture2D(filterTexture3, vec2(texel.b, .83333)).b;
    mapped.a = 1.0;
    
    gl_FragColor = v_fragmentColor * mapped;
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Sutro Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_SUTRO = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //sutroMap;
uniform sampler2D filterTexture2; //sutroMetal;
uniform sampler2D filterTexture3;  //softLight
uniform sampler2D filterTexture4; //sutroEdgeburn
//uniform sampler2D filterTexture5; //sutroCurves

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    vec2 lookup = vec2(d, texel.r);
    texel.r = texture2D(filterTexture1, lookup).r;
    lookup.y = texel.g;
    texel.g = texture2D(filterTexture1, lookup).g;
    lookup.y = texel.b;
    texel.b	= texture2D(filterTexture1, lookup).b;
    
    vec3 rgbPrime = vec3(0.1019, 0.0, 0.0);
    float m = dot(vec3(.3, .59, .11), texel.rgb) - 0.03058;
    texel = mix(texel, rgbPrime + m, 0.32);
    
    vec3 metal = texture2D(filterTexture2, v_texCoord).rgb;
    texel.r = texture2D(filterTexture3, vec2(metal.r, texel.r)).r;
    texel.g = texture2D(filterTexture3, vec2(metal.g, texel.g)).g;
    texel.b = texture2D(filterTexture3, vec2(metal.b, texel.b)).b;
    
    texel = texel * texture2D(filterTexture4, v_texCoord).rgb;
    
//    texel.r = texture2D(filterTexture5, vec2(texel.r, .16666)).r;
//    texel.g = texture2D(filterTexture5, vec2(texel.g, .5)).g;
//    texel.b = texture2D(filterTexture5, vec2(texel.b, .83333)).b;
    
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Toaster Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string FilterShaderString::FILTER_FS_TOASTER = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;  //toasterMetal
uniform sampler2D filterTexture2; //toasterSoftlight
//uniform sampler2D filterTexture3;  //toasterCurves
uniform sampler2D filterTexture4; //toasterOverlayMapWarm
uniform sampler2D filterTexture5; //toasterColorshift

void main()
{
    lowp vec3 texel;
    mediump vec2 lookup;
    vec2 blue;
    vec2 green;
    vec2 red;
    lowp vec4 tmpvar_1;
    tmpvar_1 = texture2D (CC_Texture0, v_texCoord);
    texel = tmpvar_1.xyz;
    lowp vec4 tmpvar_2;
    tmpvar_2 = texture2D (filterTexture1, v_texCoord);
    lowp vec2 tmpvar_3;
    tmpvar_3.x = tmpvar_2.x;
    tmpvar_3.y = tmpvar_1.x;
    texel.x = texture2D (filterTexture2, tmpvar_3).x;
    lowp vec2 tmpvar_4;
    tmpvar_4.x = tmpvar_2.y;
    tmpvar_4.y = tmpvar_1.y;
    texel.y = texture2D (filterTexture2, tmpvar_4).y;
    lowp vec2 tmpvar_5;
    tmpvar_5.x = tmpvar_2.z;
    tmpvar_5.y = tmpvar_1.z;
    texel.z = texture2D (filterTexture2, tmpvar_5).z;
//    red.x = texel.x;
//    red.y = 0.16666;
//    green.x = texel.y;
//    green.y = 0.5;
//    blue.x = texel.z;
//    blue.y = 0.833333;
//    texel.x = texture2D (filterTexture3, red).x;
//    texel.y = texture2D (filterTexture3, green).y;
//    texel.z = texture2D (filterTexture3, blue).z;
    mediump vec2 tmpvar_6;
    tmpvar_6 = ((2.0 * v_texCoord) - 1.0);
    mediump vec2 tmpvar_7;
    tmpvar_7.x = dot (tmpvar_6, tmpvar_6);
    tmpvar_7.y = texel.x;
    lookup = tmpvar_7;
    texel.x = texture2D (filterTexture4, tmpvar_7).x;
    lookup.y = texel.y;
    texel.y = texture2D (filterTexture4, lookup).y;
    lookup.y = texel.z;
    texel.z = texture2D (filterTexture4, lookup).z;
    red.x = texel.x;
    green.x = texel.y;
    blue.x = texel.z;
    texel.x = texture2D (filterTexture5, red).x;
    texel.y = texture2D (filterTexture5, green).y;
    texel.z = texture2D (filterTexture5, blue).z;
    lowp vec4 tmpvar_8;
    tmpvar_8.w = 1.0;
    tmpvar_8.xyz = texel;
    gl_FragColor = v_fragmentColor * tmpvar_8;
}
)";





///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Valencia Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_VALENCIA = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
uniform sampler2D filterTexture2;

mat3 saturateMatrix = mat3(
                           1.1402,
                           -0.0598,
                           -0.061,
                           -0.1174,
                           1.0826,
                           -0.1186,
                           -0.0228,
                           -0.0228,
                           1.1772);

vec3 lumaCoeffs = vec3(.3, .59, .11);

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    texel = vec3(
                 texture2D(filterTexture1, vec2(texel.r, .1666666)).r,
                 texture2D(filterTexture1, vec2(texel.g, .5)).g,
                 texture2D(filterTexture1, vec2(texel.b, .8333333)).b
                 );
    
    texel = saturateMatrix * texel;
    float luma = dot(lumaCoeffs, texel);
    texel = vec3(
                 texture2D(filterTexture2, vec2(luma, texel.r)).r,
                 texture2D(filterTexture2, vec2(luma, texel.g)).g,
                 texture2D(filterTexture2, vec2(luma, texel.b)).b);
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
}
)";





///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Walden Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_WALDEN = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
uniform sampler2D filterTexture2;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    texel = vec3(
                 texture2D(filterTexture1, vec2(texel.r, .16666)).r,
                 texture2D(filterTexture1, vec2(texel.g, .5)).g,
                 texture2D(filterTexture1, vec2(texel.b, .83333)).b);
    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    vec2 lookup = vec2(d, texel.r);
    texel.r = texture2D(filterTexture2, lookup).r;
    lookup.y = texel.g;
    texel.g = texture2D(filterTexture2, lookup).g;
    lookup.y = texel.b;
    texel.b	= texture2D(filterTexture2, lookup).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
    
}
)";




///////////////////////////////////////////////////////////////////////////////////////////////////////////////// FILTER Xproii Fragment Shader ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string FilterShaderString::FILTER_FS_XPROII = R"(
precision lowp float;

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D filterTexture1;
uniform sampler2D filterTexture2;

void main()
{
    vec3 texel = texture2D(CC_Texture0, v_texCoord).rgb;
    
    vec2 tc = (2.0 * v_texCoord) - 1.0;
    float d = dot(tc, tc);
    vec2 lookup = vec2(d, texel.r);
    texel.r = texture2D(filterTexture2, lookup).r;
    lookup.y = texel.g;
    texel.g = texture2D(filterTexture2, lookup).g;
    lookup.y = texel.b;
    texel.b	= texture2D(filterTexture2, lookup).b;
    
    vec2 red = vec2(texel.r, 0.16666);
    vec2 green = vec2(texel.g, 0.5);
    vec2 blue = vec2(texel.b, .83333);
    texel.r = texture2D(filterTexture1, red).r;
    texel.g = texture2D(filterTexture1, green).g;
    texel.b = texture2D(filterTexture1, blue).b;
    
    gl_FragColor = v_fragmentColor * vec4(texel, 1.0);
    
}
)";



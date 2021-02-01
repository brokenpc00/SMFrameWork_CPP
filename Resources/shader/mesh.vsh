attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord;

varying vec2 texCoordVarying;
varying vec4 shadingVarying;

uniform mat4 viewProjectionMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightDirection;
uniform float diffuseFactor;

void main()
{
    vec3 worldNormal = normalize(normalMatrix * a_normal);
    
    float diffuseIntensity = abs(dot(worldNormal, lightDirection));
    float diffuse = mix(1.0, diffuseIntensity, diffuseFactor);
    
    shadingVarying = vec4(diffuse, diffuse, diffuse, 1.0);
    texCoordVarying = a_texCoord;
    
    gl_PointSize = 7.0;
    gl_Position = viewProjectionMatrix * a_position;
}


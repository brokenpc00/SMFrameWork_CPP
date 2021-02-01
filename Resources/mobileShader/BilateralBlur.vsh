precision mediump float;

attribute vec4 a_position;
attribute vec2 a_texCoord;


const int GAUSSIAN_SAMPLES = 9;

uniform float texelWidthOffset;
uniform float texelHeightOffset;


varying vec2 v_texCoord;
varying vec2 blurCoordinates[GAUSSIAN_SAMPLES];

void main()
{
    gl_Position = CC_PMatrix * a_position;
    v_texCoord = a_texCoord;
    
    
    // Calculate the positions for the blur
    int multiplier = 0;
    vec2 blurStep;
    vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);
    
    for (int i = 0; i < GAUSSIAN_SAMPLES; i++)
    {
        multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));
        // Blur in x (horizontal)
        blurStep = float(multiplier) * singleStepOffset;
        blurCoordinates[i] = a_texCoord + blurStep;
    }
}

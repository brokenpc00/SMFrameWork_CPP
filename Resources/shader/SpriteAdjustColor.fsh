varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform float brightness;
uniform float contrast;
uniform float saturate;
uniform float temperature;

const vec4 ZERO = vec4(0.0, 0.0, 0.0, 0.0);

void main()
{
    vec4 textureColor = texture2D(CC_Texture0, v_texCoord);

    if (textureColor.a == 0.0) {
        gl_FragColor = ZERO;
    } else {
        // birghtness
        vec3 rgb = textureColor.rgb + vec3(brightness);

        // contrast
        rgb = (rgb - vec3(0.5)) * contrast + vec3(0.5);

        if (saturate != 1.0) {
            // saturation
            float luminance = dot(rgb, vec3(0.2125, 0.7154, 0.0721));
            rgb = mix(vec3(luminance), rgb, saturate);
        }

        if (temperature == 0.0) {
            gl_FragColor = vec4(rgb, textureColor.a) * v_fragmentColor;
        } else {
            // temperature
            float rr = rgb.r;
            float gg = rgb.g;
            float bb = rgb.b;
            vec3 processed = vec3(
                (rr < 0.5 ? (rr * 1.86) : (0.86 + 0.14*rr)),
                (gg < 0.5 ? (gg * 1.08) : (0.08 + 0.92*gg)),
                (bb < 0.5 ? 0.0 : (2.0 * bb - 1.0))
                );

            gl_FragColor = vec4(mix(rgb, processed, temperature), textureColor.a) * v_fragmentColor;
        }
    }
}



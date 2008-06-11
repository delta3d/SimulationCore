uniform sampler2D glowTexture;

varying vec4 vColor;
varying vec4 vTex0;
varying vec4 vTex1;

void main(void)
{
    vec4 texelColor0 = texture2D( glowTexture, vTex0.xy );
    vec4 texelColor1 = texture2D( glowTexture, vTex1.xy );

    gl_FragColor = mix( texelColor0, texelColor1, vColor );
}
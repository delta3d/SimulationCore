varying vec3  ModelPosition;

uniform sampler3D NoiseTexture;
uniform float Offset;
uniform vec3  ShieldColor;


void main (void)
{

    float noise  = (texture3D(NoiseTexture, ModelPosition + Offset)).a;

    noise = clamp(noise, 0.5, 1.0);
    noise -= 0.5;

    gl_FragColor = vec4(ShieldColor, noise);
}

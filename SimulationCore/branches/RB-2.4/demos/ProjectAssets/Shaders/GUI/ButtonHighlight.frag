uniform sampler2D diffuseTexture;
uniform float colorOffset;

void main (void)
{
    vec4 frag = texture2D(diffuseTexture, gl_TexCoord[0].st);
    gl_FragColor = vec4(frag.xyz, colorOffset);
}

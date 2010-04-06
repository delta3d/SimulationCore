uniform sampler2D diffuseTexture;

void main(void)
{
   gl_FragColor = texture2D(diffuseTexture, gl_TexCoord[0].xy);
}


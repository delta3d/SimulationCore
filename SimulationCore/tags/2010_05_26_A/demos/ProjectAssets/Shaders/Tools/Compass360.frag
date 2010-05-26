uniform sampler2D diffuseTexture;

void main(void)
{
	vec2 texCoord = gl_TexCoord[0].st;
	texCoord.y -= fract(0.25 * floor(texCoord.x));
   gl_FragColor = texture2D(diffuseTexture, texCoord);
}

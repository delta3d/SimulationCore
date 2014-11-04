uniform sampler2D diffuse;

void main(void)
{
   vec4 baseColor = texture2D(diffuse, gl_TexCoord[0].st); 
   gl_FragColor = baseColor;
}



varying vec4 vertexColor;

void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0] = gl_MultiTexCoord0; 
   vertexColor = gl_Color;
}

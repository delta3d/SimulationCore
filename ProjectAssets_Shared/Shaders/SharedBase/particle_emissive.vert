varying vec4 vertexColor;
varying float vDistance;

void calculateDistance(mat4, vec4, out float);

void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0] = gl_MultiTexCoord0; 
   vertexColor = gl_Color;

   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
}

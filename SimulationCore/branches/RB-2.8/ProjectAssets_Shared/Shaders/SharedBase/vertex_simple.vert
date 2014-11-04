
//this is entirely so we can pass the texture coordinates to the fragment shader
varying float vDistance;

void calculateDistance(mat4, vec4, out float);

void main(void)
{
   //Compute the final vertex position in clip space.
   gl_Position = ftransform();
   
   //Pass the texture coordinate on through.
   gl_TexCoord[0] = gl_MultiTexCoord0;

   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
}

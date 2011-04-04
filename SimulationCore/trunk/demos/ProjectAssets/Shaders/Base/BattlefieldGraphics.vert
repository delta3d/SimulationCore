
varying float vFog;

void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);


void main(void)
{
   //Compute the final vertex position in clip space.
   gl_Position = ftransform();

   float dist = 0.0;
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, dist);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, dist);  
}

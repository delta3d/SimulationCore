// This Shader is dependent on the following files
//   vertex_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

varying vec3 vNormal;
varying vec3 vLightDir; 
varying vec3 vViewDir;
varying float vFog;
varying vec3 vPosition;
varying vec4 vPosition4;

void normalizeGlNormal(out vec3);
void normalizeLight(mat4, vec4, out vec3); 
void calculateDistance(mat4, vec4, out float);

/**
 * This is the vertex shader for the interior HMMWV.  It calculates lighting, reflection, 
 * and a glare effect.  The vertex shader sets up the normal and light dir. 
 */
void main()
{
   vec4 eye4 = gl_Vertex;

   // transform normal into view space
   normalizeGlNormal(vNormal);
   normalizeLight(gl_ModelViewMatrixInverse, gl_LightSource[0].position, vLightDir);

   gl_TexCoord[0] = gl_MultiTexCoord0;
   vViewDir = normalize(-vec3(eye4) / eye4.w);
   
   gl_Position = ftransform();

   float distance;
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, distance);

   //Finally, mix the results with the fog amount.
   const float LOG2E = 1.442695; //1/log(2)
   vFog = exp2(-gl_Fog.density * gl_Fog.density * distance * distance * LOG2E);
   vFog = clamp(vFog,0.0,1.0);

   vPosition = vec3(gl_Vertex);
   vPosition4 = gl_Vertex;

}

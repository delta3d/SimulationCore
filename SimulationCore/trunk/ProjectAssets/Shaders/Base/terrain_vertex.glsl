// This Shader is dependent on the following files
//   terrain_vertex_functions.glsl
//   vertex_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

// Note - You can add about 2 more varying before it fails to compile on the 7950 Go NVidia cards
varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 vWeights; // [0] is blur, [1] is medium, and [2] is fine
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec3 worldNormal;

void normalizeGlNormal(out vec3);
void normalizeLight(mat4, vec4, out vec3);
void computeTerrainGLTexCoord(out vec4);
void computGLFogFragCoord(out float);
void computeShaderWeight(vec3, out float);
void calculateDistance(mat4, vec4, out float);
void computeWeights(float, float, float, out vec3);
void computeTerrainFog(float, float, out float);
void setTerrainGLPosition(out vec4);


//This vertex shader is meant to perform the per vertex operations of per pixel lighting
//using a single directional light source.
void main()
{
   const float MAX_DISTANCE = 750.0;
   float shaderWeight;
   
   // make normal and light dir in world space like the gl_vertex
   normalizeGlNormal(vNormal);
   normalizeLight(gl_ModelViewMatrixInverse, gl_LightSource[0].position, vLightDirection);

   //Pass the texture coordinate on through.
   computeTerrainGLTexCoord(gl_TexCoord[0]); 
   computGLFogFragCoord(gl_FogFragCoord);
   
   // reduce the effect of the detail mapping as we move away from a normal that points
   // straight up.  This prevents the shader from applying to the sides of buildings 
   // since their normal has almost no z component.
   computeShaderWeight(vNormal, shaderWeight);

   //Calculate the distance from this vertex to the camera.
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
  
   //We want the detail texture to fade away as the distance from the camera increases.
   //The detail and fine blend use curved weights based on how close they are to the camera
   computeWeights(vDistance, MAX_DISTANCE, shaderWeight, vWeights);

   //Finally, mix the results with the fog amount and compute the gl_Position
   computeTerrainFog(gl_Fog.end, vDistance, vFog);

   vec4 outGLPosTemp;   
   setTerrainGLPosition(outGLPosTemp);
   gl_Position = outGLPosTemp;
   
   vPos = gl_Vertex.xyz;
   worldNormal = gl_Normal;
}

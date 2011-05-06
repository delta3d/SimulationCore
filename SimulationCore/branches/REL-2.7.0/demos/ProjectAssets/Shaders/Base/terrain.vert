// This Shader is dependent on the following files
//   terrain_vertex_functions.glsl
//   vertex_functions.glsl
// Please be sure to list any extra files that this shader becomes dependent on.

// Note - You can add about 2 more varying before it fails to compile on the 7950 Go NVidia cards
varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 vWeights; // [0] is blur, [1] is medium, and [2] is fine
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec4 viewPos;
varying vec3 worldNormal;

uniform mat4 inverseViewMatrix;

void normalizeLight(mat4, vec4, out vec3);
void calculateDistance(mat4, vec4, out float);
void computeTerrainFog(float, float, out float);


//This vertex shader is meant to perform the per vertex operations of per pixel lighting
//using a single directional light source.
void main()
{
   const float MAX_DISTANCE = 750.0;
   float shaderWeight;
   
   // make normal and light dir in world space like the gl_vertex
   vNormal = normalize(gl_Normal);
   normalizeLight(gl_ModelViewMatrixInverse, gl_LightSource[0].position, vLightDirection);

   //Pass the texture coordinate on through.
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_FogFragCoord = gl_FogCoord;
   
   // reduce the effect of the detail mapping as we move away from a normal that points
   // straight up.  This prevents the shader from applying to the sides of buildings 
   // since their normal has almost no z component.
   shaderWeight = abs(vNormal.z);

   //Calculate the distance from this vertex to the camera.
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
  
   //We want the detail texture to fade away as the distance from the camera increases.
   //The detail and fine blend use curved weights based on how close they are to the camera
   vWeights[0] = (1.0 - clamp(vDistance/MAX_DISTANCE,0.0,1.0)) * shaderWeight;
   float blurSquared = vWeights[0] * vWeights[0];
   vWeights[1] = clamp(blurSquared * 1.3, 0.0, 1.0); // more affect closer up.
   vWeights[2] = sin(vWeights[0]) * blurSquared; // fine only effects when VERY close.

   //Finally, mix the results with the fog amount and compute the gl_Position
   computeTerrainFog(gl_Fog.end, vDistance, vFog);

   viewPos = gl_ModelViewMatrix * gl_Vertex;
   gl_Position = ftransform();
   
   //our position and normal is in local space and we want it it
   vPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
      inverseViewMatrix[1].xyz,
      inverseViewMatrix[2].xyz);

   worldNormal = inverseView3x3 * gl_NormalMatrix * gl_Normal;
}

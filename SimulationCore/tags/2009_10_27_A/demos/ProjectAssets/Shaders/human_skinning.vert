
/*
 *  This shader provides hardware skinning and is used in dtAnim
 *  Bradley Anderegg
 */

const int MAX_BONES = 30;
uniform vec4 boneTransforms[MAX_BONES * 3];
uniform mat4 inverseViewMatrix;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec3 worldNormal;

void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);

void main(void)
{
   //initialize our data
   vec4 transformedPosition = vec4(0.0, 0.0, 0.0, 1.0);
   vec4 transformedNormal = vec4(0.0, 0.0, 0.0, 1.0);
   float boneWeights[4];//float[](gl_MultiTexCoord2.x, gl_MultiTexCoord2.y, gl_MultiTexCoord2.z, gl_MultiTexCoord2.w);
   float boneIndices[4];//int[](gl_MultiTexCoord3.x, gl_MultiTexCoord3.y, gl_MultiTexCoord3.z, gl_MultiTexCoord3.w);
   
   boneWeights[0] = gl_MultiTexCoord2.x; boneWeights[1] = gl_MultiTexCoord2.y; boneWeights[2] = gl_MultiTexCoord2.z; boneWeights[3] = gl_MultiTexCoord2.w;
   boneIndices[0] = gl_MultiTexCoord3.x; boneIndices[1] = gl_MultiTexCoord3.y; boneIndices[2] = gl_MultiTexCoord3.z; boneIndices[3] = gl_MultiTexCoord3.w;


   //multiply each bone and weight to get the offset matrix
   for(int i = 0; i < 3; ++i)
   {
      int boneIndex = int(boneIndices[i] * 3.0);
      float boneWeight = boneWeights[i];

      vec4 bt0 =  boneTransforms[boneIndex];
      vec4 bt1 =  boneTransforms[boneIndex + 1];
      vec4 bt2 =  boneTransforms[boneIndex + 2];

      transformedPosition.x += boneWeight * dot(gl_Vertex, bt0);
      transformedPosition.y += boneWeight * dot(gl_Vertex, bt1);
      transformedPosition.z += boneWeight * dot(gl_Vertex, bt2);

      transformedNormal.x += boneWeight * dot(gl_Normal.xyz, bt0.xyz);
      transformedNormal.y += boneWeight * dot(gl_Normal.xyz, bt1.xyz);
      transformedNormal.z += boneWeight * dot(gl_Normal.xyz, bt2.xyz);
   }

   //set proper varyings
   gl_TexCoord[0] = gl_MultiTexCoord0;

   //Normals of the Vertex and the LightDirection calculated for use in the fragment shader
   vNormal = transformedNormal.xyz;
   vLightDir = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));

   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, vDistance);  
   
   //our position and normal is in local space and we want it it
   vPos = vec3(inverseViewMatrix * gl_ModelViewMatrix * transformedPosition);
   
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
      inverseViewMatrix[1].xyz,
      inverseViewMatrix[2].xyz);

   worldNormal = inverseView3x3 * gl_NormalMatrix * transformedNormal.xyz;

   gl_Position = gl_ModelViewProjectionMatrix * transformedPosition;
}


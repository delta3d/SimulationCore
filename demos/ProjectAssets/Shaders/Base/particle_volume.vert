const int MAX_NUM_PARTICLES = 150;

uniform vec4 volumeParticlePos[MAX_NUM_PARTICLES];
uniform mat4 inverseViewMatrix;

varying vec4 vPos;
varying vec4 vOffset;
varying vec3 vRadius;
varying mat3 vNormalMatrix;

void main()
{
   //offset.xyz stores the local offset relative to the volume, offset.w stores the radius
   vOffset = volumeParticlePos[int(gl_Vertex.w)];
   vec3 center = gl_ModelViewMatrix[3].xyz + vOffset.xyz;
   vec4 pos = vec4(vOffset.w * gl_Vertex.x, vOffset.w * gl_Vertex.y, vOffset.w * gl_Vertex.z, 1.0);
   
   //now rotate to align with eye
   mat4 viewMat = inverseViewMatrix * gl_ModelViewMatrix;
   
   mat3 rotateToEye;
   rotateToEye[1] = normalize(inverseViewMatrix[3].xyz - center);
   rotateToEye[2] = viewMat[2].xyz;
   rotateToEye[0] = cross(rotateToEye[1], rotateToEye[2]);

   pos.xyz = rotateToEye * pos.xyz;
   pos.xyz += vOffset.xyz + gl_ModelViewMatrix[3].xyz;

   vPos = pos;
   gl_Position = gl_ProjectionMatrix * vPos;

   vNormalMatrix = gl_NormalMatrix;
   vRadius = vec3(gl_MultiTexCoord0.xy, vOffset.w);
}


const int MAX_NUM_PARTICLES = 500;

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
   
   vec4 pos = vec4(vOffset.xyz, 1.0);
   vec4 ePos = gl_ModelViewMatrix * pos;
   ePos.xy += gl_Vertex.xy * vOffset.w;
   vPos = ePos;
   gl_Position = gl_ProjectionMatrix * vPos;

   vNormalMatrix = gl_NormalMatrix;
   vRadius = vec3(gl_MultiTexCoord0.xy, vOffset.w);
}


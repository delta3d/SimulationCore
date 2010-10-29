const int MAX_NUM_PARTICLES = 150;

uniform vec4 volumeParticlePos[MAX_NUM_PARTICLES];
uniform float volumeParticleRadius;

uniform mat4 inverseViewMatrix;

varying vec4 vPos;
varying vec4 vOffset;
varying vec2 vTexCoords;
varying mat3 vNormalMatrix;

void main()
{
   vOffset = volumeParticlePos[int(gl_Vertex.w)];
   
   vec4 pos = vec4(vOffset.xyz, 1.0);
   vec4 ePos = gl_ModelViewMatrix * pos;
   ePos.xy += gl_Vertex.xy * volumeParticleRadius;
   vPos = ePos;
   gl_Position = gl_ProjectionMatrix * vPos;

   vNormalMatrix = gl_NormalMatrix;
   vTexCoords = gl_MultiTexCoord0.xy;
}


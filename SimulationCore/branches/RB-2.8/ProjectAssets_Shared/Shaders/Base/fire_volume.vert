const int MAX_NUM_PARTICLES = 150;

uniform vec4 volumeParticlePos[MAX_NUM_PARTICLES];
uniform float volumeParticleRadius;
uniform float volumeParticleDensity;

uniform mat4 inverseViewMatrix;

varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;
varying vec3 vPos;

void main()
{
   //vParticleOffset = vec3(gl_Vertex.w, gl_Vertex.w*1.4, gl_Vertex.w*0.5);
   vOffset = volumeParticlePos[int(gl_Vertex.w)];
   
   vViewPosCenter = gl_ModelViewMatrix * vec4(vOffset.xyz, 1.0);
   vec3 center_pos = (inverseViewMatrix * vViewPosCenter).xyz;
   
   vViewPosVert = vViewPosCenter;
   vViewPosVert.xy += gl_Vertex.xy * volumeParticleRadius;

   vPos = (inverseViewMatrix * vViewPosVert).xyz;
   gl_Position = gl_ProjectionMatrix * vViewPosVert;
 
   vTexCoords = gl_MultiTexCoord0.xy;
}


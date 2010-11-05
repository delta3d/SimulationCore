const int MAX_NUM_PARTICLES = 150;

uniform vec4 volumeParticlePos[MAX_NUM_PARTICLES];
uniform float volumeParticleRadius;

uniform mat4 inverseViewMatrix;

varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec3 vLightContrib;
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;

varying vec3 vNormal;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);

void main()
{
   vOffset = volumeParticlePos[int(gl_Vertex.w)];
   
   vViewPosCenter = gl_ModelViewMatrix * vec4(vOffset.xyz, 1.0);
   vec3 center_pos = (inverseViewMatrix * vViewPosCenter).xyz;
   
   vViewPosVert = vViewPosCenter;
   vViewPosVert.xy += gl_Vertex.xy * volumeParticleRadius;
   vec4 worldPos = inverseViewMatrix * vViewPosVert;
   gl_Position = gl_ProjectionMatrix * vViewPosVert;
 
   vTexCoords = gl_MultiTexCoord0.xy;


   // Create a normal that faces the camera (in view space), change to world space
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);
   vec3 outwardNormal = inverseView3x3 * vec3(0.0, 0.0, 1.0);
   // Create a normal facing diagonally out at each vertex.
   vNormal = normalize(worldPos.xyz - center_pos);
   // Now combine the two to get a slightly angled, 45 degrees away. 
   vNormal = (vNormal + outwardNormal)/2.0;
   vNormal = normalize(vNormal);
 
   //Compute the Light Contribution
   vLightContrib = vec3(1.0, 1.0, 1.0);

   vec3 vLightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), vLightContrib);


   vec3 dynamicLightContrib;
   vec3 spotLightContrib;
   dynamic_light_fragment(vNormal, worldPos.xyz, dynamicLightContrib);
   spot_light_fragment(vNormal, worldPos.xyz, spotLightContrib);
   
   vLightContrib = vLightContrib + (dynamicLightContrib) + (spotLightContrib);
   vLightContrib = clamp(vLightContrib, 0.0, 1.0);
}


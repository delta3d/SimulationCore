const int MAX_NUM_PARTICLES = 150;

uniform vec4 volumeParticlePos[MAX_NUM_PARTICLES];
uniform float volumeParticleRadius;

uniform mat4 inverseViewMatrix;

varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec3 vLightContrib;
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;

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

   //Compute the Light Contribution
   vec3 normal = normalize(worldPos.xyz - center_pos);
   vLightContrib = vec3(1.0, 1.0, 1.0);

   vec3 dynamicLightContrib;
   vec3 dynamicLightContribUp;
   vec3 spotLightContrib;
   vec3 spotLightContribUp;
   vec3 Up = vec3(0.0, 0.0, 1.0);

   vec3 lightDir = normalize((inverseViewMatrix * gl_LightSource[0].position).xyz);

   float diffuseContrib = max(dot(normal, lightDir), 0.0);
   float upContrib = max(dot(Up, lightDir), 0.0);

   vLightContrib = gl_LightSource[0].ambient.rgb + (gl_LightSource[0].diffuse.rgb * (diffuseContrib + upContrib));


   dynamic_light_fragment(normal, worldPos.xyz, dynamicLightContrib);
   dynamic_light_fragment(Up, worldPos.xyz, dynamicLightContribUp);
   spot_light_fragment(normal, worldPos.xyz, spotLightContrib);
   spot_light_fragment(Up, worldPos.xyz, spotLightContribUp);
   
   vLightContrib = vLightContrib + (0.5 * (dynamicLightContrib + dynamicLightContribUp)) + (0.5 * (spotLightContrib + spotLightContribUp));
   vLightContrib = clamp(vLightContrib, 0.0, 1.0);
}


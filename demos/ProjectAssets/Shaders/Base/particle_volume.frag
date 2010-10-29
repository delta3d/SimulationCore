uniform sampler2D depthTexture;
uniform sampler3D noiseTexture;
uniform float osg_SimulationTime;
uniform mat4 inverseViewMatrix;

uniform vec4 volumeParticleColor;
uniform float volumeParticleRadius;
uniform float volumeParticleIntensity;
uniform vec3 volumeParticleVelocity;

varying vec4 vPos;
varying vec4 vOffset;
varying vec2 vTexCoords;
varying mat3 vNormalMatrix;


float computeFog(float startFog, float endFog, float fogDistance)
{
   float fogTemp = pow(2.0, (fogDistance - startFog) / (endFog - startFog)) - 1.0;
   return clamp(fogTemp, 0.0, 1.0);
}

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);

void main(void)
{

   vec3 normal;
   normal.xy = vTexCoords.xy * 2.0 - vec2(1.0, 1.0);
   float r = dot(normal.xy, normal.xy);
   normal.z = -sqrt(1.0 - r);
   normal = vec3(normal.x, normal.z, normal.y);

   if(r > 1.0) discard;

   float sceneDepth = texture2D(depthTexture, gl_FragCoord.xy).r;

   vec3 noiseCoords =  normal + (vOffset.xyz) + vec3(0.0, 0.0, 0.25 * osg_SimulationTime);
   float noise  = abs( (texture3D(noiseTexture, noiseCoords)).a );
   
    if(noise < r)
        discard;
    else
        noise -= r;

   vec4 pixelPos = vec4(vPos.xyz + (normal * vec3(volumeParticleRadius)), 1.0);
   vec4 clipSpacePos = gl_ModelViewProjectionMatrix * pixelPos;
   float depth = abs(clipSpacePos.z / clipSpacePos.w);

   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
   inverseViewMatrix[1].xyz,
   inverseViewMatrix[2].xyz);

   //normal = inverseView3x3 * vNormalMatrix * normal;

   //Compute the Light Contribution
   vec3 lightContrib = vec3(1.0, 1.0, 1.0);
   /*vec3 dynamicLightContrib;
   vec3 spotLightContrib;
   vec3 lightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);
   
   lightContribution(normal, lightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);  
   dynamic_light_fragment(normal, inverseViewMatrix[3].xyz, dynamicLightContrib);
   spot_light_fragment(normal, inverseViewMatrix[3].xyz, spotLightContrib);
   
   lightContrib = lightContrib + dynamicLightContrib + spotLightContrib;
   lightContrib = clamp(lightContrib, 0.0, 1.0);*/

   float fogDist = abs(sceneDepth - depth);//, 0.0, 1.0);
   float fogAmt = computeFog(0.0, 0.1, 10.0 * fogDist);
   gl_FragColor = vec4(lightContrib * volumeParticleColor.xyz, fogAmt * volumeParticleColor.w * volumeParticleIntensity * noise * noise);
   //gl_FragColor = vec4(normal, 1.0);  
   //gl_FragDepth = depth;
}


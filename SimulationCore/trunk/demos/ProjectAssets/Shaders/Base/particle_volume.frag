uniform sampler2D depthTexture;
uniform sampler3D noiseTexture;
uniform float osg_SimulationTime;

uniform vec4 volumeParticleColor;
uniform float volumeParticleRadius;
uniform float volumeParticleIntensity;
uniform vec3 volumeParticleVelocity;

varying vec4 vWorldPos;
varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec3 vLightContrib;
varying vec3 vLightDir;
varying vec3 vNormal;


float computeFog(float startFog, float endFog, float fogDistance)
{
   float fogTemp = pow(2.0, (fogDistance - startFog) / (endFog - startFog)) - 1.0;
   return clamp(fogTemp, 0.0, 1.0);
}

void main(void)
{

   vec3 normal;
   normal.xy = vTexCoords.xy * 2.0 - vec2(1.0, 1.0);
   float r = dot(normal.xy, normal.xy);
   normal.z = -sqrt(1.0 - r);
   normal = vec3(normal.x, normal.z, normal.y);

   if(r > 1.0) discard;

   float sceneDepth = texture2D(depthTexture, gl_FragCoord.xy).r;

   vec3 noiseCoords =  normal + (vOffset.xyz) + (osg_SimulationTime * volumeParticleVelocity);
   float noise  = abs( (texture3D(noiseTexture, noiseCoords)).a );
   
    if(noise < r)
        discard;
    else
        noise -= r;

   vec4 pixelPos = vec4(vWorldPos.xyz + (vNormal * vec3(volumeParticleRadius)), 1.0);
   vec4 clipSpacePos = gl_ModelViewProjectionMatrix * pixelPos;
   float depth = abs(clipSpacePos.z / clipSpacePos.w);


   float fogDist = 0.1;//abs(sceneDepth - depth);//, 0.0, 1.0);
   float fogAmt = computeFog(0.05, 0.1, 10.0 * fogDist);
   gl_FragColor = min(r + r + noise, 1.0) * vec4(vLightContrib * volumeParticleColor.xyz, fogAmt * volumeParticleColor.w * volumeParticleIntensity * noise);
   //gl_FragDepth = depth;
}


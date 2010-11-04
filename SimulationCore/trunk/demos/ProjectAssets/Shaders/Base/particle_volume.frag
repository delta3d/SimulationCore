uniform sampler2D depthTexture;
uniform sampler3D noiseTexture;
uniform float osg_SimulationTime;
uniform float nearPlane;
uniform float farPlane;

uniform vec4 volumeParticleColor;
uniform float volumeParticleRadius;
uniform float volumeParticleIntensity;
uniform float volumeParticleDensity;
uniform vec3 volumeParticleVelocity;
uniform vec2 ScreenDimensions;

varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec3 vLightContrib;
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;


float computeFog(vec3 viewPosCenter, vec3 viewPosCurrent, float radius, vec2 screenCoord)
{
   float opacity = 0.0;
   float dist = length(viewPosCenter.xy - viewPosCurrent.xy);
   if(dist < radius)
   {
      float vpLength = radius + length(viewPosCurrent);
      float fMin = nearPlane * vpLength / viewPosCurrent.z;
      float w = sqrt(radius * radius - dist * dist);
      float f = vpLength - w;
      float b = vpLength + w;
      float sceneDepth = texture2D(depthTexture, screenCoord).r;
      sceneDepth *= (farPlane - nearPlane);
      float ds = min(sceneDepth, b) - max(fMin, f);
      float sphereDepth = (1.0 - dist / radius) * ds;
      opacity = 1.0 - exp(-volumeParticleDensity * sphereDepth);
   }

   return opacity;
}

void main(void)
{

   vec3 normal;
   normal.xy = vTexCoords.xy * 2.0 - vec2(1.0, 1.0);
   float r = dot(normal.xy, normal.xy);
   normal.z = -sqrt(1.0 - r);
   normal = vec3(normal.x, normal.z, normal.y);

   if(r > 1.0) discard;

   vec3 noiseCoords =  normal + (vOffset.xyz) + (osg_SimulationTime * volumeParticleVelocity);
   float noise  = abs( (texture3D(noiseTexture, noiseCoords)).a );
   
    if(noise < r)
        discard;
    else
        noise -= r;

   float fogAmt = computeFog(vViewPosCenter.xyz, vViewPosVert.xyz, volumeParticleRadius, gl_FragCoord.xy / ScreenDimensions);
   gl_FragColor = min(r + r + noise, 1.0) * vec4(vLightContrib * volumeParticleColor.xyz, fogAmt * volumeParticleColor.w * volumeParticleIntensity * noise);
}


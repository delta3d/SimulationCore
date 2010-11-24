uniform sampler3D noiseTexture;
uniform float osg_SimulationTime;
uniform float nearPlane;
uniform float farPlane;

uniform vec4 volumeParticleColor;
uniform float volumeParticleRadius;
uniform float volumeParticleIntensity;
uniform float volumeParticleDensity;
uniform float volumeParticleVelocity;
uniform float volumeParticleNoiseScale;
uniform vec2 ScreenDimensions;

varying vec4 vOffset;
varying vec2 vTexCoords;
varying vec3 vLightContrib;
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;
varying vec3 vParticleOffset;

varying vec3 vPos;
varying vec3 vNormal;

float softParticleOpacity(vec3 viewPosCenter, vec3 viewPosCurrent, 
      float radius, vec2 screenCoord, float density);


void main(void)
{

   vec2 radiusPosition = vTexCoords.xy * 2.0 - vec2(1.0, 1.0);
   // use r*r (vice r) using a dot, which drops the square root of length(). Works well.
   float r = dot(radiusPosition.xy, radiusPosition.xy); 
   if(r > 1.0) discard; // Eliminate processing on boundary pixels

   vec3 noiseCoords = 0.5 /** (0.25 - vOffset.w)*/ * vec3(vec2(radiusPosition.xy), 1.0);
   noiseCoords.xy += vPos.xy / 150.0;
   float noise = (texture3D(noiseTexture, noiseCoords)).a;
   noise = 20.0 * pow(noise, 15.0);
   
   //if(noise < 2.5 * vOffset.w * r) discard;
   //else noise -= pow(r, 2.0);

   // soft particles avoid sharp cuts into main geometry
   float opacity = softParticleOpacity(vViewPosCenter.xyz, vViewPosVert.xyz, 
         volumeParticleRadius, gl_FragCoord.xy / ScreenDimensions, volumeParticleDensity);

   vec3 finalColor = volumeParticleColor.xyz;
   //vec3 lightNormal = vec3(particleNoiseOffset.x, particleNoiseOffset.y, 0.0);
   //lightNormal = normalize(lightNormal);
   //float lightValue = dot(vNormal, lightNormal);
   //finalColor *= lightValue;
   float fadeOut = 1.0 - length(vLightContrib);
   //fadeOut = pow(fadeOut, 3.0);
   float finalAlpha = 0.1 * opacity * volumeParticleColor.w * volumeParticleIntensity * noise * fadeOut * length(finalColor);
   gl_FragColor = vec4(finalColor,finalAlpha);
   //gl_FragColor = vec4(vec3(noise), 1.0);
}


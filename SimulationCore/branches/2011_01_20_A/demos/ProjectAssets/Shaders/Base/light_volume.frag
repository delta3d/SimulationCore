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

   vec3 noiseCoords = 0.610 * vec3(vec2(radiusPosition.xy), 1.0);
   //noiseCoords.xy += vPos.xy / 150.0;
   //noiseCoords.z = mod(osg_SimulationTime, 5.0) + vOffset.z * (0.00015 * osg_SimulationTime);
   noiseCoords.z = 1000.0 + mod(volumeParticleVelocity * osg_SimulationTime, 1000.0)+ vOffset.z;// * (10000.0 + mod(osg_SimulationTime, 5000.0));
   float noise = (texture3D(noiseTexture, noiseCoords)).a;
   noise = 2.5 * pow(noise, 8.0);
   
   // soft particles avoid sharp cuts into main geometry
   float opacity = softParticleOpacity(vViewPosCenter.xyz, vViewPosVert.xyz, 
         volumeParticleRadius, gl_FragCoord.xy / ScreenDimensions, volumeParticleDensity);

   vec3 finalColor = volumeParticleColor.xyz;
   float fadeOut = (0.5 + (1.0 - length(vLightContrib)) / 1.5);
   float finalAlpha = opacity * volumeParticleColor.w * volumeParticleIntensity * noise * fadeOut * length(finalColor);
   gl_FragColor = vec4(finalColor,finalAlpha);
}


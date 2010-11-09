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

   vec3 noiseCoords = vNormal*volumeParticleRadius/4.0 + (vOffset.xyz) + (osg_SimulationTime * volumeParticleVelocity);
   float noise = (texture3D(noiseTexture, noiseCoords)).a;
   
   float partialAlpha = volumeParticleColor.w * volumeParticleIntensity * (noise * pow(1.0-r, 0.333));
   if(partialAlpha < 0.1) //noise < r
      discard; // make right at the end disappear - but softly. 

   // soft particles avoid sharp cuts into main geometry
   float opacity = softParticleOpacity(vViewPosCenter.xyz, vViewPosVert.xyz, 
         volumeParticleRadius, gl_FragCoord.xy / ScreenDimensions, volumeParticleDensity);

   // use some noise in final color to keep it interesting (straight noise makes it too dark).
   vec3 finalColor = (0.6 + 0.6*noise) * vLightContrib * volumeParticleColor.xyz;
   float finalAlpha = opacity * partialAlpha;
   gl_FragColor = vec4(finalColor,finalAlpha);
}


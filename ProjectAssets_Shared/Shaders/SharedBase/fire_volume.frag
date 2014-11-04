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
varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;
varying vec3 vParticleOffset;
varying vec3 vPos;

float softParticleOpacity(vec3 viewPosCenter, vec3 viewPosCurrent, 
      float radius, vec2 screenCoord, float density);


void main(void)
{

   vec2 radiusPosition = vTexCoords.xy * 2.0 - vec2(1.0, 1.0);
   float r = dot(radiusPosition.xy, radiusPosition.xy); 

   float particleNoiseScaleOffset = mod(vOffset.w, 0.13);
   vec2 particleNoiseOffset = vec2(sin(vOffset.w), -cos(vOffset.w));
   vec3 noiseCoords = vec3(vec2(radiusPosition.xy + particleNoiseOffset), 1.0);
   if(r > 0.25) noiseCoords.z = osg_SimulationTime * (0.5 + pow(1.0 - r, 15.0));
   else noiseCoords.z = osg_SimulationTime * 0.25;

   float noise = (texture3D(noiseTexture, noiseCoords)).a;
   noise *= 2.0;
   noise -= 1.0;
   if(noise <  r) discard;
   
   float fireNoiseColor = noise + sin(noise - r);
   fireNoiseColor *= 5.0 * r;

   //float fireNoiseColor2 = max(1.0 - abs(sin(20.0 * fireNoiseColor * radiusPosition.x) - cos(20.0 * fireNoiseColor * radiusPosition.y)), 0.0);
   float fireNoiseColor2 = (1.5 + r) * fireNoiseColor;
   fireNoiseColor2 -= floor(fireNoiseColor2);

   vec3 fireColorCenter = vec3(0.5, 0.5, 0.75);
   vec3 fireColorStart = vec3(1.0, 1.0, 0.0);
   vec3 fireColorEnd = vec3(1.0, 0.5, 0.0);

   noise = max(noise, 0.75);
   vec3 finalFireColor = (1.0 - r) * ((fireNoiseColor * fireColorEnd) + ( (1.0 - fireNoiseColor) * fireColorStart ));
   finalFireColor += vec3(fireNoiseColor2 * fireColorEnd);
   finalFireColor += 2.5 * noise * pow((1.0 - r), 15.0) * fireColorCenter;
   

   // soft particles avoid sharp cuts into main geometry
   float opacity = softParticleOpacity(vViewPosCenter.xyz, vViewPosVert.xyz, 
         volumeParticleRadius, gl_FragCoord.xy / ScreenDimensions, 0.25);

   // use some noise in final color to keep it interesting (straight noise makes it too dark).
   vec3 finalColor = finalFireColor;
   float finalAlpha = opacity * volumeParticleColor.w * volumeParticleIntensity * noise;
   gl_FragColor = vec4(finalColor,finalAlpha);
   //gl_FragColor = vec4(vec3(noise), 1.0);
   //gl_FragColor = vec4(vec3(fireNoiseColor2), 1.0);
}


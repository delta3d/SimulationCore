//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
//The wave parameters are packed into two vec4's like so
// [Length, Speed, Amplitude, Frequency], [Q, reserved for later use, Direction.x, Direction.y]
//////////////////////////////////////////////////////////////////////////////////////////////////
const int WAVE_OFFSET = 0;
const int NUMWAVES = 8;
const int MAX_WAVES = 8;
const float NEAR_FADE_DIST = 20.0;
const float MID_FADE_DIST = 200.0;
const float FAR_FADE_DIST = 1000.0;
uniform vec4 waveArray[2 * MAX_WAVES];
uniform float WaterHeight;// = 443.0;

uniform sampler2D waveTexture;
uniform sampler2D reflectionMap;
//uniform samplerCube reflectionCubeMap;

uniform float ScreenHeight;
uniform float ScreenWidth;
uniform float waveDirection;
	

varying vec4 pos;
//varying vec4 viewDir;
varying vec3 vertexNormal;
//varying vec4 camPos;
varying vec3 lightVector;
varying float distanceScale;
varying vec2 vFog;
varying float distBetweenVertsScalar;

uniform float elapsedTime;
uniform mat4 inverseViewMatrix;
uniform float modForFOV;

vec3 waterColor = vec3(10.0 / 256.0, 69.0 / 256.0, 39.0 / 256.0); 
vec3 deepWaterColor = 0.74 * waterColor;  

vec3 skyColor = vec3(0.40, 0.6322, 0.8652);  


vec2 rotateTexCoords(vec2 coords, float angle)
{
   float degInRad = radians(angle);   
   
   vec2 coordsRot;
   coordsRot.x = dot(vec2(cos(degInRad), -sin(degInRad)), coords);
   coordsRot.y = dot(vec2(sin(degInRad), cos(degInRad)), coords);
   return coordsRot;
}

float FastFresnel(float nDotL, float fbias, float fpow)
{
   float facing = 1.0 - nDotL;
   return max(fbias + ((1.0 - fbias) * pow(facing, fpow)), 0.0);
}

vec3 SampleNormalMap(sampler2D tex, vec2 texCoords)
{
   vec4 color = texture2D(tex, texCoords);
   color *= 2.0;
   color -= 1.0;
   return normalize(color.xyz);
}

float edgeFade(float blendStart, vec2 texCoord)
{
   texCoord = mod(texCoord, 1.0);   
   float dx = (0.5 - texCoord.x);
   float dy = (0.5 - texCoord.y);
 
   float dist = clamp(0.5 - length(vec2(dx, dy)), 0.0, 0.5);      

   float fadeAmt = clamp(dist - blendStart, 0.0,  blendStart) / blendStart;
   return fadeAmt;
}

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
vec3 GetWaterColorAtDepth(float);


void main (void)
{   
   vec3 camPos = inverseViewMatrix[3].xyz;
   vec3 combinedPos = pos.xyz + camPos;
   vec2 localCoord = vec2(combinedPos.xy - camPos.xy);

   float camDistance = length(combinedPos.xyz - camPos.xyz);

   float fadeTransition = 0.05;
   vec3 waveNormal = vec3(0.0, 0.0, 0.0); 
   vec2 waveCoords = vec2(combinedPos.xy / 40.0);   
   waveCoords = rotateTexCoords(waveCoords, waveDirection);

   float fadeAmt = edgeFade(fadeTransition, waveCoords);
   waveNormal += fadeAmt * SampleNormalMap(waveTexture, waveCoords);

   vec2 waveCoords2 = vec2(0.5, 0.5) + waveCoords;
   float fadeAmt2 = (1.0 - fadeAmt) * edgeFade(fadeTransition, waveCoords2);
   waveNormal += fadeAmt2 * SampleNormalMap(waveTexture, waveCoords2);

   vec2 waveCoords3 = vec2(0.25, 0.25) + waveCoords;
   float fadeAmt3 = 1.0 - clamp(fadeAmt + fadeAmt2, 0.0, 1.0);
   waveNormal += fadeAmt3 * SampleNormalMap(waveTexture, waveCoords3);
   
   //waveNormal /= 3.0;

   vec3 shaderVertexNormal = vec3(0.0, 0.0, 1.0);// = normalize(vertexNormal);


   ////////////////////////////////////////////
   //// Curt Test
   float zModifier = 0.0;
   // There are 2 vec4's of data per wave, so the loop is MAX_WAVES * 2 but increments by 2's
   for(int i = WAVE_OFFSET; i < NUMWAVES * 2 + WAVE_OFFSET; i+=2)
   {           
      float waveLen = waveArray[i].x;
      float speed = waveArray[i].y;
      float freq = waveArray[i].w;
      float amp = waveArray[i].z;
      vec2 waveDir = waveArray[i + 1].zw;
      //maxHeightPercent += amp + 0.000001;
      
      // scale out small waves as we get too far away
      //float wavePulseScalar = 1.0 / waveLen;
      //float wavePulseFactor = 1.0 - clamp(1.25 * sin(dot(waveDir.yx, pos.xy) * 0.2), 0.0, 1.0);
      //amp *= /*wavePulseFactor * */(1.0 - clamp(distBetweenVertsScalar / waveLen, 0.0, 0.999));
      amp *= 1.0 - clamp((distBetweenVertsScalar) / (waveLen), 0.0, 0.999);

      float mPlusPhi =  (freq * (speed * elapsedTime + combinedPos.x * waveDir.x + waveDir.y * combinedPos.y));       
      float k = max(waveArray[i+1].x, 1.00001);

      float sinPhi = sin(mPlusPhi);// + max(0.0, 2.0 * sin((dot(waveDir.yx, pos.xy)) / 5.0)));
      //cos/sin of the sum of the previous two variables
      float sinDir = pow((sinPhi + 1.0) / 2.0, k);
      
      // Yeah, you just try to delete the 0.00001. You go booooyyyy! Hacktastic
      zModifier += 0.000001 + amp * sinDir;
      //maxHeightPercent += sinDir; // sinDir is the amp height contribution %
     
      float vertexDerivativeScalar = freq * amp * pow((sinPhi + 1.0) * 0.5, k - 1.0) * cos(mPlusPhi);
      shaderVertexNormal.x += k * waveDir.x * vertexDerivativeScalar;
      shaderVertexNormal.y += k * waveDir.y * vertexDerivativeScalar;
      
   }   

   // Curt - Question - What about the pos.w??? 
   vec4 finalPos = vec4(combinedPos.x, combinedPos.y, WaterHeight + zModifier, 1.0);//pos.w);
   vec4 viewDir = (finalPos - inverseViewMatrix[3]);

   shaderVertexNormal.x = -shaderVertexNormal.x;
   shaderVertexNormal.y = -shaderVertexNormal.y;
   shaderVertexNormal = normalize(shaderVertexNormal);
   //shaderVertexNormal.xy /= NUMWAVES;
   //vec3 vertexNormal = vec3(0.0, 0.0, 1.0); //normalize(VertexNormal);
   waveNormal.z *= shaderVertexNormal.z;
   waveNormal = normalize(waveNormal);
   vec3 finalVertexNormal = 0.5 * (normalize(shaderVertexNormal) + waveNormal);
   //vec3 finalVertexNormal = waveNormal;
   //vec3 finalVertexNormal = normalize(waveNormal);
   /////////////////////////////////


   if(gl_FrontFacing)
   {     
      vec3 normal = /*waveNormal + 2.0 **/ finalVertexNormal;
      //vec3 normal = finalVertexNormal;
      normal = normalize(normal);
      float textureDetailScalar = distanceScale;
      normal = mix(shaderVertexNormal, normal, textureDetailScalar);
      normal = normalize(normal);

      vec4 eye = normalize(-viewDir);
      //vec4 eye = normalize(combinedPos - camPos);
      float waveNDotL = max(0.0, dot(eye.xyz, normal));

      //float fresnel = FastFresnel(waveNDotL, 0.07, 3.0);
      float fresnel = FastFresnel(waveNDotL, 0.17, 3.0);

      vec3 refTexCoords = vec3(gl_FragCoord.x / ScreenWidth, gl_FragCoord.y / ScreenHeight, gl_FragCoord.z);
      refTexCoords.xy = clamp(refTexCoords.xy + (0.4 * normal.xy), 0.0, 1.0);
      vec3 reflectColor = texture2D(reflectionMap, refTexCoords.xy).rgb;
      reflectColor = 0.8 * reflectColor;

      vec3 lightVect = normalize(lightVector);
      
      vec3 lightContribFinal;
      lightContribution(normal, lightVector, gl_LightSource[0].diffuse.xyz, 
         gl_LightSource[0].ambient.xyz, lightContribFinal);
      lightContribFinal = pow(lightContribFinal, 0.5);

      vec3 waterColorContrib = mix(waterColor, deepWaterColor, waveNDotL);
      waterColorContrib *= lightContribFinal;

      fresnel = pow(fresnel, 3.0);
      reflectColor = mix(waterColorContrib, reflectColor, fresnel);
      
      vec3 normRefLightVec = reflect(lightVect, normal);
      float specularContrib = max(0.0, dot(normRefLightVec, vec3(-eye)));
      specularContrib = (0.2 * pow(specularContrib, 4.0)) + (0.25 * pow(specularContrib, 28.0));
      //specularContrib += 1200.0 * pow(specularContrib, 100.0);
      vec3 specularResult = vec3(gl_LightSource[0].specular.xyz * specularContrib);     
      
      vec3 finalColor = mix(reflectColor + specularResult, gl_Fog.color.rgb, vFog.x);
      gl_FragColor = vec4(finalColor, 1.0);
      //gl_FragColor = vec4((normal+1.0)*0.5, 1.0);
      
      //gl_FragColor = vec4(vec3(distanceScale), 1.0);;//vec4(vec3((VertexNormal + 1.0) * 0.5), 1.0);
   }
   else
   {
      finalVertexNormal.z = -finalVertexNormal.z;
      vec3 normal = waveNormal + 2.0 * finalVertexNormal;
      normal = normalize(normal);
      float textureDetailScalar = distanceScale;
      normal = mix(finalVertexNormal, normal, textureDetailScalar);
      normal = normalize(normal);

      vec4 eye = normalize(-viewDir);
      //vec4 eye = normalize(combinedPos - camPos);
      float waveNDotL = max(0.0, dot(eye.xyz, normal));     
      
      float fresnel = FastFresnel(waveNDotL, 0.137, 2.0);

      vec3 waterColorAtDepth = GetWaterColorAtDepth(camPos.z);

      vec3 resultColor = (0.5 * fresnel) + mix(waterColor, waterColorAtDepth, fresnel);

      vec3 lightContribFinal;
      lightContribution(vec3(0.0, 0.0, 1.0), lightVector, gl_LightSource[0].diffuse.xyz, 
         gl_LightSource[0].ambient.xyz, lightContribFinal);
      //lightContribFinal = pow(lightContribFinal, 0.5);
      //resultColor *= lightContribFinal;

      float alpha = clamp(vFog + 0.95, 0.0, 1.0);      
      vec3 combinedColor = lightContribFinal * mix(resultColor, waterColorAtDepth, vFog.y);
      gl_FragColor = vec4(combinedColor, 1.0);
   }
}

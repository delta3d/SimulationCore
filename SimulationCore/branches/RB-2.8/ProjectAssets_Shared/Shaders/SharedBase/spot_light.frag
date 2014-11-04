
const int MAX_SPOT_LIGHTS = 3;
const int NUM_SPOT_LIGHTS_TO_USE = 3;
const int NUM_SPOT_LIGHT_ATTRIBS = 4;

//each spot light has 4 associated vec4's
//the first vec4 is a vec3 position and an intensity
//the second vec4 is a vec3 color
//the third vec4 is a vec3 attenuation and a float spot exponent
//the fourth is a vec3 direction and float cos cutoff
uniform vec4 spotLights[NUM_SPOT_LIGHT_ATTRIBS * MAX_SPOT_LIGHTS];

// NOTE - Commented out Reflective for performance reasons        
void spot_light_fragment(vec3 normal, vec3 pos, out vec3 totalLightContrib)
{
   totalLightContrib = vec3(0.0, 0.0, 0.0);  

   for(int i = 0; i < NUM_SPOT_LIGHTS_TO_USE * NUM_SPOT_LIGHT_ATTRIBS; i += NUM_SPOT_LIGHT_ATTRIBS)
   {      
      float spotCosCutoff = spotLights[i + 3].w;
      float spotExponent = spotLights[i + 2].w;
      vec3 spotPosition = vec3(spotLights[i].xyz);
      vec3 spotDirection = normalize(spotLights[i + 3].xyz);
      
      vec3 lightDir = spotPosition - pos;
      float dist = length(lightDir);
      float dist2 = dist * dist;
      lightDir /= dist;
      
      float spotEffect = dot(spotDirection, -lightDir);
      bool spotToggle = spotEffect > spotCosCutoff;
      float absIntensity = max(0.0, spotLights[i].w); //the intensity can be negative to support shadows on terrain
                                                      //but we have to clamp it here to keep from messing everything else up

      spotEffect = max(0.0, (pow(spotEffect, spotExponent)));//just in case we want a negative cutoff we will do an abs here

      //this computes the attenuation which keeps the positional lights lighting within range of the light
      float attenDenom = spotLights[i + 2].x + (spotLights[i + 2].y * dist) + (spotLights[i + 2].z * dist2);
      float atten = min(1.0, spotEffect / attenDenom);
      float normalDotLight = max(0.0, dot(normal, -spotDirection));//lightDir));

      //we use 65% of the dot product lighting contribution and then add 35% of the ambient contribution
      //which is basically taken as just the light color
      vec3 dotProductLightingAndAmbient = spotLights[i+1].xyz * (0.65 * normalDotLight + 0.35);
         
      //we attenuate the resulting contribution of the dot product and ambient lighting,
      //multiply it by the intensity and then accumulate it into the resulting color

      // we don't add anything sometimes
      if (spotToggle && absIntensity > 0.0)
      {
         totalLightContrib += absIntensity * atten * dotProductLightingAndAmbient; 
      }
   } 
   
   totalLightContrib = clamp(totalLightContrib, 0.0, 1.0);
}

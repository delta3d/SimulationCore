
const int MAX_DYNAMIC_LIGHTS = 5;
const int NUM_DYNAMIC_LIGHTS_TO_USE = 5;

//each dynamic light has 3 associated vec4's
//the first vec4 is a vec3 position and an intensity
//the second vec4 is a vec3 color
//and the third vec4 is a vec3 attenuation and a saturation intensity
uniform vec4 dynamicLights[3 * MAX_DYNAMIC_LIGHTS];

// NOTE - Commented out Reflective for performance reasons        
void dynamic_light_fragment(vec3 normal, vec3 pos, out vec3 totalLightContrib)
{
   totalLightContrib = vec3(0.0,0.0, 0.0);

   for(int i = 0; i < NUM_DYNAMIC_LIGHTS_TO_USE * 3; i+=3)
   {      
     //if(dynamicLights[i].w > 0.000001) // Note - if's are terrible on the GPU and this ran slower.
      vec3 lightDir = vec3(dynamicLights[i].xyz) - pos;
      float dist = length(lightDir);      
      float dist2 = dist * dist;
      
      //this computes the attenuation which keeps the positional lights lighting within range of the light
      float atten = 1.0 / ( dynamicLights[i + 2].x + (dynamicLights[i + 2].y * dist) + (dynamicLights[i + 2].z * dist2));      
      float normalDotLight = max(0.0, dot(normal, normalize(lightDir)));
            
      //we use 50% of the dot product lighting contribution and then add 50% of the ambient contribution
      //which is basically taken as just the light color
      vec3 dotProductLightingAndAmbient = 0.5 * ((1.0 + normalDotLight) * dynamicLights[i+1].xyz);
         
      //we attenuate the resulting contribution of the dot product and ambient lighting,
      //multiply it by the intensity and then accumulate it into the resulting color
      float absIntensity = max(0.0, dynamicLights[i].w); //the intensity can be negative to support shadows on terrain
                                                         //but we have to clamp it here to keep from messing everything else up
      totalLightContrib +=  absIntensity * min(1.0, atten) * dotProductLightingAndAmbient; 
   } 
   
   totalLightContrib = clamp(totalLightContrib, 0.0, 1.0);
}

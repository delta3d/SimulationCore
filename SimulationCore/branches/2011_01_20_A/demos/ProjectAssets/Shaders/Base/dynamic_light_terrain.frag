
const int MAX_DYNAMIC_LIGHTS = 20;
const int NUM_DYNAMIC_LIGHTS_TO_USE = 6;

//each dynamic light has 3 associated vec4's
//the first vec4 is a vec3 position and an intensity
//the second vec4 is a vec3 color
//and the third vec4 is a vec3 attenuation and a saturation intensity
uniform vec4 dynamicLights[3 * MAX_DYNAMIC_LIGHTS];

// NOTE - Commented out Reflective for performance reasons        
void dynamic_light_fragment(vec3 normal, vec3 pos, out vec3 totalLightContrib, out vec3 reflectiveDynamicLightContrib)
{

   totalLightContrib = vec3(0.0,0.0, 0.0);
   reflectiveDynamicLightContrib = vec3(0.0, 0.0, 0.0);

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
      // We don't need ambient for terrain, only for vehicles
      vec3 dotProductLightingAndAmbient = normalDotLight * dynamicLights[i+1].xyz;
         
      //we attenuate the resulting contribution of the dot product and ambient lighting,
      //multiply it by the intensity and then accumulate it into the resulting color
      totalLightContrib += dynamicLights[i].w * atten * dotProductLightingAndAmbient; 

      //we calculate a close up saturation effect by using 20% of the linear attenuation with the distance squared
      //a linear attentuation of 0.005 will saturate about 10 meters
      //dynamicLights[i + 2].w  is the intensity of the saturation effect
      //reflectiveDynamicLightContrib += dynamicLights[i + 2].w * (normalDotLight * (0.2 / (dynamicLights[i + 2].y * dist2)));
   } 
   
   //totalLightContrib = clamp(totalLightContrib, 0.0, 1.0);
   //reflectiveDynamicLightContrib = clamp(reflectiveDynamicLightContrib, 0.0, 1.0);
}

//////////////////////////////////////////////
//An under water shader
//by Bradley Anderegg
//////////////////////////////////////////////

uniform float WaterHeight;
uniform mat4 inverseViewMatrix;
uniform vec3 waterHeightScreenSpace;

varying vec4 worldSpacePos;
varying vec3 lightVector;
varying vec4 camPos;
varying float height;

float deepWaterScalar = 0.5;
float viewDistance = 10.0;

uniform vec4 WaterColor;
vec4 deepWaterColor = deepWaterScalar * WaterColor;

vec3 GetWaterColorAtDepth(float);
void lightContribution(vec3, vec3, vec3, vec3, out vec3);

void main (void)
{     

   float depth = worldSpacePos.y - waterHeightScreenSpace.y;//WaterHeight - (viewDistance * worldSpacePos.z);
   depth = clamp(depth, 0.0, viewDistance);
   float depthScalar = (depth / viewDistance);

   vec3 lightContribFinal;
   lightContribution(vec3(0.0, 0.0, 1.0), lightVector, gl_LightSource[0].diffuse.xyz, 
      gl_LightSource[0].ambient.xyz, lightContribFinal);

   
   vec3 color = lightContribFinal * GetWaterColorAtDepth(camPos.z);
   color = mix(color, deepWaterColor.xyz, 0.5 * depthScalar);

   //this is a little hacky but it is used to remove the line above the water
   float dist = (WaterHeight + 0.35) - camPos.z;
   dist = clamp(dist, -0.005, 0.005);

   if(worldSpacePos.y < (waterHeightScreenSpace.y + dist))
   {
      //gl_FragColor = vec4(vec3(lightContribFinal), 1.0);      
      gl_FragColor = vec4(color, 1.0);     
   }
   else
   {
      discard;
      //gl_FragColor = vec4(color, 0.0);   
   }
}

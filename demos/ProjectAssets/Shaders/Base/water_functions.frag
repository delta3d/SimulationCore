
uniform float WaterHeight;
uniform vec4 WaterColor;

const float cDeepWaterScalar = 0.74;
const float cViewDistance = 100.0; 
const vec3 cDeepWaterColor = cDeepWaterScalar * WaterColor;


vec3 GetWaterColorAtDepth(float pDepth)
{
   float dist = WaterHeight - pDepth;
   dist = clamp(dist, 0.0, cViewDistance);
   float depthScalar = (dist / cViewDistance);

   vec3 color = mix(WaterColor.xyz, cDeepWaterColor, depthScalar);

   return color;
}

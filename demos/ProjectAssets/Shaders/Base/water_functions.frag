
uniform float WaterHeight;

const float cDeepWaterScalar = 0.74;
const float cViewDistance = 100.0;
const vec3 cWaterColor = vec3(10.0 / 256.0, 69.0 / 256.0, 39.0 / 256.0); 
const vec3 cDeepWaterColor = cDeepWaterScalar * cWaterColor;


vec3 GetWaterColorAtDepth(float pDepth)
{
   float dist = WaterHeight - pDepth;
   dist = clamp(dist, 0.0, cViewDistance);
   float depthScalar = (dist / cViewDistance);

   vec3 color = mix(cWaterColor, cDeepWaterColor, depthScalar);

   return color;
}

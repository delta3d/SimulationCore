// This Shader is dependent on the following files
//   fragment_functions.glsl
//   terrain_fragment_functions.glsl
// Please be sure to list any extra files that this shader becomes dependent on.

uniform sampler2D baseTexture;
uniform sampler2D secondaryTexture; 

uniform float WaterHeight;

varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 vWeights;
varying vec2 vFog;
varying float vDistance;
varying vec3 vPos;
varying vec3 worldNormal;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void computeDetailBlend(sampler2D, vec2, vec3, out float); 
void computeTerrainColor(vec3, vec4, float, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
vec3 GetWaterColorAtDepth(float);

const float UnderWaterViewDistance = 5.0;

void main(void)
{   
   //Compute the Light Contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDirection, gl_LightSource[0].diffuse.xyz, gl_LightSource[0].ambient.xyz, lightContrib);
   
   //Compute Detail Additive
   float detailBlend;
   computeDetailBlend(secondaryTexture, gl_TexCoord[0].st, vWeights, detailBlend); 

   //Computes the Base Color
   vec4 baseColor = texture2D(baseTexture, gl_TexCoord[0].st);
   
   vec3 color = lightContrib * (vec3(baseColor) + vec3(detailBlend)); 
   color = clamp(color, 0.0, 1.0);
 
   //start fogging out at a depth of 0.15 meters below sea level
   float height = WaterHeight - 0.15;
   float depth = height - vPos.z;

   vec3 lightContribUnderWaterFog;
   lightContribution(vec3(0.0, 0.0, 1.0), vLightDirection, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContribUnderWaterFog);  
   vec3 underWaterColor = lightContribUnderWaterFog * GetWaterColorAtDepth(depth);  

   depth = clamp(depth, 0.0, UnderWaterViewDistance) / UnderWaterViewDistance;

   //Discard the fragment outside some bound so that we don't get the jagged edges where the terrain ends
   if(vDistance > 10000.0 || baseColor.a < 1.0)
   {
      discard;
   }
   else
   {
      vec3 color2 = mix(mix(color, gl_Fog.color.rgb, vFog.x), mix(color, underWaterColor, pow(gl_FragCoord.z, 2.0)), depth);
      gl_FragColor = vec4(vec3(color2), 1.0);

   }
}

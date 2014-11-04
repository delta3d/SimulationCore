// This Shader is dependent on the following files
//   fragment_functions.glsl
//   terrain_fragment_functions.glsl
// Please be sure to list any extra files that this shader becomes dependent on.

uniform sampler2D baseTexture;
uniform sampler2D secondaryTexture; 
uniform bool writeLinearDepth;

varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 vWeights;
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec4 viewPos;
varying vec3 worldNormal;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void computeDetailBlend(sampler2D, vec2, vec3, out float); 
void computeTerrainColor(vec3, vec4, float, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);
float computeFragDepth(float, float);

void main(void)
{   
   vec3 ecPosition3 = viewPos.xyz / viewPos.w;
   float dist = length(ecPosition3);
   float fragDepth = computeFragDepth(dist, gl_FragCoord.z);
   gl_FragDepth = fragDepth;

   //currently we only write a linear depth when doing a depth pre-pass
   //as an optimization we return immediately afterwards
   if(writeLinearDepth)
   {
      return;
   }

   //Compute the Light Contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDirection, gl_LightSource[0].diffuse.xyz, gl_LightSource[0].ambient.xyz, lightContrib);
   
   //Compute Detail Additive
   float detailBlend;
   computeDetailBlend(secondaryTexture, gl_TexCoord[0].st, vWeights, detailBlend); 

   //Computes the Base Color
   vec4 baseColor = texture2D(baseTexture, gl_TexCoord[0].st);

   vec3 dynamicLightContrib;
   dynamic_light_fragment(worldNormal, vPos, dynamicLightContrib);

   vec3 spotLightContrib;
   spot_light_fragment(worldNormal, vPos, spotLightContrib);
   dynamicLightContrib += spotLightContrib;

   lightContrib += dynamicLightContrib;
   lightContrib = clamp(lightContrib, 0.0, 1.0);
   
   vec3 color = lightContrib * (vec3(baseColor) + vec3(detailBlend)); 
   color = clamp(color, 0.0, 1.0);

   //Mix the final color with the fog and don't forget the alpha
   alphaMix(color, gl_Fog.color.rgb, vFog, baseColor.a, gl_FragColor);   
}

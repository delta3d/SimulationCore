// This Shader is dependent on the following files
//   fragment_functions.glsl
//   shader_gloss_fragment_functions.glsl
//   shader_vehicle_fragment_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

uniform sampler2D reflectTexture;
uniform sampler2D glossTexture;
uniform sampler2D diffuseTexture;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vViewDir;
varying vec2 vReflectTexCoord;
varying float vFog;
varying float vDistance;

void create2DTexture(sampler2D, vec2, out vec4);
void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void computeBaseVehicleColor(vec3, vec4, out vec3);
void computeSpecularContribution(vec3, vec3, vec3, vec3, out vec3);
void computeGlossColor(vec3, vec3, vec3, vec3, inout vec3);
void alphaMix(vec3, vec3, float, float, out vec4);

void main(void)
{

   //Computes the Diffuse Color
   vec4 diffuseColor;
   create2DTexture(diffuseTexture, gl_TexCoord[0].st, diffuseColor);
   
   //Compute the light contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);
   
   vec4 tempGlossMap;
   vec3 glossMap;// = vec3(texture2D(glossTexture,gl_TexCoord[0].st));
   create2DTexture(glossTexture, gl_TexCoord[0].st, tempGlossMap);
   glossMap = vec3(tempGlossMap);
   
   vec4 tempReflectMap;
   vec3 reflectMap;// = vec3(texture2D(reflectTexture,vReflectTexCoord));
   create2DTexture(reflectTexture, vReflectTexCoord, tempReflectMap);
   reflectMap = vec3(tempReflectMap);

   //Compute the specular contribution
   //vec3 reflectVec = reflect(vLightDir, vNormal);
   //float reflectContrib = max(0.0,dot(reflectVec, -vViewDir));
   vec3 specularContrib;// = vec3(glossMap.r) * (pow(reflectContrib, 16.0));
   computeSpecularContribution(vLightDir, vNormal, vViewDir, glossMap, specularContrib);
   
   //Compute the glossy color for the vehicle
   vec3 color;
   computeBaseVehicleColor(lightContrib, diffuseColor, color);
   computeGlossColor(reflectMap, lightContrib, glossMap, specularContrib, color);					

   if( vDistance > 10000.0)
   {
      discard;
   }
   else
   {
      alphaMix(color, gl_Fog.color.rgb, vFog, diffuseColor.a, gl_FragColor);
   }
}


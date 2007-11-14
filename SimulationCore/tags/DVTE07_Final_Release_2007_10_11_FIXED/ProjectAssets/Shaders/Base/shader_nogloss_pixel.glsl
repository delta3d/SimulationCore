// This Shader is dependent on the following files
//   fragment_functions.glsl
//   shader_vehicle_fragment_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

uniform sampler2D diffuseTexture;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec3 worldNormal;

void create2DTexture(sampler2D, vec2 , out vec4);
void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void computeBaseVehicleColor(vec3, vec4, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3 normal, vec3 pos, out vec3 totalLightContrib, out vec3 reflectiveDynamicLightContrib);


void main(void)
{
   //Computes the Diffuse Color
   vec4 diffuseColor;
   create2DTexture(diffuseTexture, gl_TexCoord[0].st, diffuseColor);
   
   //Compute the Light Contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);
   
   vec3 dynamicLightContrib;
   vec3 reflectiveDynamicLightContrib;
   dynamic_light_fragment(worldNormal, vPos, dynamicLightContrib, reflectiveDynamicLightContrib);
   lightContrib += dynamicLightContrib;
   lightContrib = clamp(lightContrib, 0.0, 1.0);
   
   //Compute the color of the pixel
   vec3 color;
   computeBaseVehicleColor(lightContrib, diffuseColor, color);
   color += reflectiveDynamicLightContrib;
   color = clamp(color, 0.0, 1.0);
   
   if( vDistance > 10000.0)
   {
      discard;
   }
   else
   {
      //Mix the final color with the fog and don't forget the alpha
      alphaMix(color, gl_Fog.color.rgb, vFog, diffuseColor.a, gl_FragColor);
   }
}


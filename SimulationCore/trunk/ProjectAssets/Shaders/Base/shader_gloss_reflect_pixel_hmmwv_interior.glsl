// This Shader is dependent on the following files
//   fragment_functions.glsl
//   shader_gloss_fragment_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

uniform sampler2D diffuseTexture;
uniform sampler2D detailTexture;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vViewDir;
varying float vFog;
varying vec3 worldPos;
varying vec3 worldNormal;

void create2DTexture(sampler2D, vec2, out vec4);
void interiorLightContribution(vec3, vec3, out vec3);
void computeSpecularContribution(vec3, vec3, vec3, vec3, out vec3);
void dynamic_light_fragment(vec3 normal, vec3 pos, out vec3 totalLightContrib, out vec3 reflectiveDynamicLightContrib);

/**
 * This is the fragment shader for the interior HMMWV
 */
void main(void)
{

   vec3 color = vec3(0.0, 0.0, 0.0);
   
   vec4 diffuseColor;
   create2DTexture(diffuseTexture, gl_TexCoord[0].st, diffuseColor);

   // Compute Detail Additive - use the R, G, and B channels of the detail texture.  
   // The B is high level bluring, R is for mid level details, and G is for fine details.
   // Sample the texel for each channel and blend it in depending on each levels blend weight
   // Before applying, normalize the color value to -0.5 < channel < 0.5 instead of 0 < x < 1.0.
   // This keeps us from making the image brighter cause each color channel is centered around 127.
   // Use the UVW text lookup value. It's not great, but better than spherical or eye text-gen.
   vec2 textCoord = vec2(gl_TexCoord[0].st);
   float blurBlend = texture2D(detailTexture, textCoord * 6.0).b - 0.40;
   float mediumBlend = texture2D(detailTexture, textCoord * 0.25).r - 0.40;
   float fineBlend = texture2D(detailTexture, textCoord * 1.0).g - 0.40;
   float detailBlend = 0.2 * (blurBlend / 5.0 + fineBlend / 5.0 + mediumBlend / 7.0);

   // Light contribution considers the light impacting the surface. Since that 
   // is too dramatic, we weight the effect of light against a straight up vector.
   vec3 lightContrib;
   interiorLightContribution(vNormal, vLightDir, lightContrib);
   
   //accumulate the dynamic light contribution
   vec3 dynamicLightContrib;
   vec3 reflectiveDynamicLightContrib;
   dynamic_light_fragment(worldNormal, worldPos, dynamicLightContrib, reflectiveDynamicLightContrib);
   lightContrib += dynamicLightContrib;
   lightContrib = clamp(lightContrib, 0.0, 1.0);

   color += lightContrib * vec3(diffuseColor.rgb + detailBlend); 

   //Specular...
   vec3 specularContrib;
   computeSpecularContribution(vLightDir, vNormal, vViewDir, vec3(0.1), specularContrib);
   
   color += specularContrib * lightContrib;  // specular value is not set on the light :(
   
   //add in the dynamic light reflective
   color += reflectiveDynamicLightContrib;
   color = clamp(color, 0.0, 1.0);

   if (diffuseColor.a < 0.3) {
      // Give the window a bit of a 'glare' effect if the light is mostly directly on.  
      float winReflect = max(0.0, dot(vLightDir, -vViewDir));
      float winSpecular = pow(winReflect, 8.0);

      winSpecular *= lightContrib + 0.2;
      gl_FragColor = vec4(vec3(1.7 * winSpecular), pow(diffuseColor.a + 0.9*winSpecular, 2.0));
   }
   else
      gl_FragColor = vec4(color.xyz, 1.0);
}


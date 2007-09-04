uniform sampler2D baseTexture;
uniform sampler2D secondaryTexture;

//varying vec2 testXY;
varying vec3 vNormal;
varying vec3 vLightDirection;
varying vec3 weights; // [0] is blur, [1] is medium, and [2] is fine
//varying float blurBlendWeight;
//varying float medBlendWeight;
//varying float fineBlendWeight;
varying float fog;

varying float distance;

void main(void)
{
   float blurBlendWeight = weights[0];
   float medBlendWeight = weights[1];
   float fineBlendWeight = weights[2];
   vec4 baseColor = texture2D(baseTexture, gl_TexCoord[0].st);
   vec3 normal = normalize(vNormal);
   vec3 lightDir = normalize(vLightDirection);

   // Light contribution considers the light impacting the surface. Since that 
   // is too dramatic, we weight the effect of light against a straight up vector.
   float diffuseSurfaceContrib = max(dot(normal, lightDir),0.0);
   float fUpContribution = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
   float diffuseContrib = fUpContribution * 0.42 + diffuseSurfaceContrib * 0.62;

   // Lit Color (Diffuse plus Ambient)
   vec3 diffuseLight = vec3(gl_LightSource[0].diffuse) * diffuseContrib;
   vec3 lightContrib = diffuseLight + vec3(gl_LightSource[0].ambient);

   // Compute Detail Additive - use the R, G, and B channels of the detail texture.  
   // The B is high level bluring, R is for mid level details, and G is for fine details.
   // Sample the texel for each channel and blend it in depending on each levels blend weight
   // Before applying, normalize the color value to -0.5 < channel < 0.5 instead of 0 < x < 1.0.
   // This keeps us from making the image brighter cause each color channel is centered around 127.
   // We repeat the textures 4, 8, 32 times the base texture to give it different effects at runtime.
   
   // this attempts to use the XY coord, but it has a HUGE seam/  Also, it is more hatched and applies 
   // the blend to the buildings and roads VERY noticably.
   //vec2 textCoord = mod(testXY, 200.0)/200.0; 
   
   vec2 textCoord = gl_TexCoord[0].st;
   vec3 blurBlend = (vec3(texture2D(secondaryTexture, textCoord * 4.0)) - 0.48) * blurBlendWeight;
   vec3 medBlend = (vec3(texture2D(secondaryTexture, textCoord * 8.0)) - 0.48) * medBlendWeight;
   vec3 fineBlend = (vec3(texture2D(secondaryTexture, textCoord * 32.0)) - 0.48) * fineBlendWeight;
   float detailBlend2 = (blurBlend.b * 1.8) + (medBlend.r/1.7) + fineBlend.g * 1.2;

   //vec3 result = vec3(mod(testXY.x, 100.0)/100.0, mod(testXY.y, 100.0)/100.0, 1.0);
   // compute final color with light, fog, base color, and detail
   vec3 result = lightContrib * (vec3(baseColor) + vec3(detailBlend2)); 

   // Discard the fragment outside some bound so that we don't get the jagged edges where the terrain ends
   // Note - This distnace should be passed in.
   if( distance > 10000.0)
   {
      discard;
   }
   else
   {
      vec3 mixColor = mix(result, gl_Fog.color.rgb, fog);
      gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);;//vec4(mixColor.rgb, baseColor.a);
   }

}

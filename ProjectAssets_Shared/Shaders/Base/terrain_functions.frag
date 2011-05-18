// The following shaders make use of it least one of this functions:
//   terrain_fragment.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)

/////////////////////////////////////////////////////////////////////
//////////////////Start Terrain Specific Functions///////////////////
/////////////////////////////////////////////////////////////////////

void computeDetailBlend(sampler2D secondaryTexture,vec2 textCoord, vec3 weights, out float detailBlend)
{
   // Compute Detail Additive - use the R, G, and B channels of the detail texture.  
   // The B is high level bluring, R is for mid level details, and G is for fine details.
   // Sample the texel for each channel and blend it in depending on each levels blend weight
   // Before applying, normalize the color value to -0.5 < channel < 0.5 instead of 0 < x < 1.0.
   // This keeps us from making the image brighter cause each color channel is centered around 127.
   // We repeat the textures 4, 8, 32 times the base texture to give it different effects at runtime.
   vec3 blurBlend = (vec3(texture2D(secondaryTexture, textCoord * 4.0)) - 0.48) * weights[0];
   vec3 medBlend = (vec3(texture2D(secondaryTexture, textCoord * 8.0)) - 0.48) * weights[1];
   vec3 fineBlend = (vec3(texture2D(secondaryTexture, textCoord * 32.0)) - 0.48) * weights[2];
   detailBlend = (blurBlend.b * 1.8) + (medBlend.r/1.7) + fineBlend.g * 1.2;
}

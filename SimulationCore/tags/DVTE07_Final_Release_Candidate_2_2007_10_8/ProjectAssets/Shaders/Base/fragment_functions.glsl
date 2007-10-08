// The following shaders make use of it least one of this functions:
//   ephemeris_fog_frag.glsl
//   perpixel_lighting_one_directional_frag.glsl
//   perpixel_lighting_detailmap_frag.glsl
//   shader_gloss_reflect_pixel.glsl
//   shader_gloss_reflect_pixel_hmmwv_interior.glsl
//   shader_nogolss_pixel.glsl
//   terrain_fragment.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
///////////General Functions Used By Fragment Fog Shaders////////////
/////////////////////////////////////////////////////////////////////

void alphaMix(vec3 color1, vec3 color2, float fogContrib, float alpha, out vec4 mixColor)
{
   mixColor = vec4( mix(color1, color2, fogContrib), alpha);
}

void lightContribution(vec3 normal, vec3 lightDir, vec3 diffuseLightSource, vec3 ambientLightSource, out vec3 lightContrib)
{
   // Light contribution considers the light impacting the surface. Since that 
   // is too dramatic, we weight the effect of light against a straight up vector.
   float diffuseSurfaceContrib = max(dot(normal, lightDir),0.0);
   float fUpContribution = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
   float diffuseContrib = fUpContribution * 0.42 + diffuseSurfaceContrib * 0.62;

   // Lit Color (Diffuse plus Ambient)
   vec3 diffuseLight = diffuseLightSource * diffuseContrib;
   lightContrib = vec3(diffuseLight + ambientLightSource);
}

void interiorLightContribution(vec3 normal, vec3 lightDir, out vec3 lightContrib)
{
   // Light contribution considers the light impacting the surface. Since that 
   // is too dramatic, we weight the effect of light against a straight up vector.
   float diffuseSurfaceContrib = max(dot(normal, lightDir),0.0);
   float fUpContribution = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
   float diffuseContrib = fUpContribution * 0.62 + diffuseSurfaceContrib * 0.42;

   // Lit Color (Diffuse plus Ambient)
   vec3 diffuseLight = vec3(gl_LightSource[0].diffuse) * diffuseContrib;
   lightContrib = diffuseLight + vec3(gl_LightSource[0].ambient);
}

void create2DTexture(sampler2D baseTexture, vec2 texCoord, out vec4 outVec)
{
   outVec = texture2D(baseTexture, texCoord);
}
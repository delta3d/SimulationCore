// The following shaders make use of it least one of this functions:
//   shader_gloss_reflect_pixel.glsl
//   shader_gloss_reflect_pixel_hmmwv_interior.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
///////////////Start Gloss Vehicle Specific Functions////////////////
/////////////////////////////////////////////////////////////////////

void computeSpecularContribution(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 glossMap, out vec3 specularContribution)
{
   vec3 reflectVec = reflect(lightDir, normal);
   float reflectContrib = max(0.0,dot(reflectVec, -viewDir));
   specularContribution = vec3(glossMap.r) * (pow(reflectContrib, 16.0));
}



void computeGlossColor(vec3 reflectMap, vec3 lightContrib, vec3 glossMap, vec3 specularContrib, inout vec3 color)
{
   // Mix in the sky reflection depending on reflect value specified in G channel 
   // Note - for flat surfaces, the reflection works horibly.
   color = mix(color,reflectMap, min(lightContrib, glossMap.g));

   // don't apply specular greater than the light contrib or we get glowing trucks in the dark...
   color += min(specularContrib, lightContrib);	
}
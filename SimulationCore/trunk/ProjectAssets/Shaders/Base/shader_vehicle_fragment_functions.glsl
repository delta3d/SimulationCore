// The following shaders make use of it least one of this functions:
//   shader_gloss_reflect_pixel.glsl
//   shader_nogolss_pixel.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
//////////////////Start Vehicle Specific Functions///////////////////
/////////////////////////////////////////////////////////////////////

void computeBaseVehicleColor(vec3 lightContrib, vec4 diffuseColor, out vec3 color)
{
   color = lightContrib * vec3(diffuseColor);
}
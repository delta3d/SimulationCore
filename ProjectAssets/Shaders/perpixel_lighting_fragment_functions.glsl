// The following shaders make use of it least one of this functions:
//   perpixel_lighting_one_directional_frag.glsl
//   perpixel_lighting_detailmap_frag.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
////////////General Functions Used By Per Pixel Shaders//////////////
/////////////////////////////////////////////////////////////////////

void initilizePerPixelVariables(out vec3 color, inout vec3 normal, inout vec3 lightDir, out float diffuseContrib)
{
   normal = normalize(normal);
   lightDir = normalize(lightDir);
   color = vec3(gl_LightSource[0].ambient); 
   diffuseContrib = max(dot(lightDir,normal),0.0);
}

void computeColor(vec3 vHalfVec, vec3 vNormal, inout vec3 color)
{
   //For now this constant is defined..  this value should, however, be set
   //and used through gl_LightSource...
   const vec3 SPECULAR_COLOR = vec3(0.3,0.3,0.3);

   vec3 halfV = normalize(vHalfVec);
   color += SPECULAR_COLOR * pow(max(0.0,dot(vNormal,halfV)),16.0);
}
// The following shaders make use of it least one of this functions:
//   perpixel_lighting_one_directional_vert.glsl
//   perpixel_lighting_detailmap_vert.glsl


// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

void computePerPixelLighting(out vec3 normal, out vec3 lightDir, out vec3 halfVec, out vec4 GLTexCoord, out vec4 GLPosition)
{
   normal = gl_NormalMatrix * gl_Normal;
   lightDir = normalize(vec3(gl_LightSource[0].position));
   halfVec = normalize(vec3(gl_LightSource[0].halfVector));

   GLTexCoord = gl_TextureMatrix[0] * gl_MultiTexCoord0;
   GLPosition = ftransform();
}
// The following shaders make use of it least one of this functions:
//   shader_gloss_reflect_vertex.glsl
//   shader_gloss_reflect_vertex_hmmwv_interior.glsl
//   shader_nogloss_vertex.glsl
//   shader_vehicle_vertex_functions.glsl
//   terrain_vertex.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
////////////General Functions Used By Vertex Fog Shaders/////////////
/////////////////////////////////////////////////////////////////////

void calculateDistance(mat4 modelViewMatrix, vec4 vertex, out float dist)
{
   vec4 ecPosition = modelViewMatrix * vertex;
   vec3 ecPosition3 = vec3(ecPosition) / ecPosition.w;
   dist = length(ecPosition3);
}

void normalizeLight(mat4 InverseMVM, vec4 position, out vec3 lightNormal)
{
   lightNormal = normalize(vec3(InverseMVM * position));
}

void normalizeGlNormal(out vec3 normal)
{
   normal = normalize(gl_Normal);
}

float computeFog(float startFog, float endFog, float fogDistance)
{
   float fogTemp = pow(2.0, (fogDistance - startFog) / (endFog - startFog)) - 1.0;
   return clamp(fogTemp, 0.0, 1.0);
}

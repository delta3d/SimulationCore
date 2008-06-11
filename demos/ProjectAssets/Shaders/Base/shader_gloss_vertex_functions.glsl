// The following shaders make use of it least one of this functions:
//   shader_gloss_reflect_vertex.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
///////////////Start Gloss Vehicle Specific Functions////////////////
/////////////////////////////////////////////////////////////////////

/**
 * Calculates a set of spherical texture coordinates for sampling
 * from a spherical environment map.
 */
void sphereMap(in vec3 eye, in vec3 normal, out vec2 ReflectTexCoord)
{
   float m;
   vec3 r,u;

   u = normalize(eye);
   r = reflect(u,normal);
   m = 2.0 * sqrt(dot(r.xy,r.xy) + ((r.z + 1.0) * (r.z + 1.0)));

   ReflectTexCoord = vec2(r.x/m + 0.5,r.y/m + 0.5);
}

void normalizedViewDir(mat4 modelViewMatrix, vec4 vertex, out vec3 viewDirection)
{
   vec4 eye4 = modelViewMatrix * vertex;
   viewDirection = normalize(-vec3(eye4) / eye4.w);
}
 
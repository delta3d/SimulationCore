// This Shader is dependent on the following files
//   vertex_functions.glsl

// The following shaders make use of it least one of this functions:
//   shader_gloss_reflect_vertex.glsl
//   shader_nogloss_vertex.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

float computeFog(float, float, float);

/////////////////////////////////////////////////////////////////////
//////////////////Start Vehicle Specific Functions///////////////////
/////////////////////////////////////////////////////////////////////

void computeVehicleGLTexCoord(out vec4 texCoord0)
{
   texCoord0 = gl_MultiTexCoord0;
}

void computeVehicleFog(float endFog, float fogDistance, out float fog)
{
   fog = computeFog(endFog * 0.15, endFog, fogDistance);
}

void setVehicleGLPosition(out vec4 position)
{
   position = ftransform();
}

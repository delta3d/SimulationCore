// This Shader is dependent on the following files
//   perpixel_lighting_vertex_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

varying vec3 vNormal;
varying vec3 vHalfVec;
varying vec3 vLightDir;

void computePerPixelLighting(out vec3, out vec3, out vec3, out vec4, out vec4);

/**
 * This is the vertex shader to a simple set of shaders that calculates
 * per pixel lighting assuming one directional light source.
 */
void main()
{
   vec4 outGLPosTemp;
   computePerPixelLighting(vNormal, vLightDir, vHalfVec, gl_TexCoord[0], outGLPosTemp);
   gl_Position = outGLPosTemp;       
}

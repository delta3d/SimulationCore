// This Shader is dependent on the following files
//   shader_gloss_vertex_functions.glsl
//   shader_vehicle_vertex_functions.glsl
//   vertex_functions.glsl

// Please be sure to list any extra files that this shader becomes dependent on.
// Thanks :)  
// -Matthew "w00by" Stokes

uniform mat4 inverseViewMatrix;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vViewDir;
varying vec2 vReflectTexCoord;
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec3 worldNormal;

void computeVehicleGLTexCoord(out vec4);
void normalizedViewDir(mat4, vec4, out vec3);
void normalizeLight(mat4, vec4, out vec3);
void normalizeGlNormal(out vec3);
void sphereMap(vec3, vec3, out vec2);
void calculateDistance(mat4, vec4, out float);
void computeVehicleFog(float, float, out float);
void setVehicleGLPosition(out vec4);

/**
 * This is the vertex shader to a simple set of shaders that calculates
 * per pixel lighting assuming one directional light source.  It also
 * calculates texture coordinates suitable for doing sphere map reflection.
 */
void main()
{
   //gl_TexCoord[0] stored for use in the fragment shader
   computeVehicleGLTexCoord(gl_TexCoord[0]);
   
   //Calculates the normalized view direction and stores it for use in the fragment shader
   normalizedViewDir(gl_ModelViewMatrix, gl_Vertex, vViewDir);

   // make normal and light dir in world space like the gl_vertex
   normalizeLight(gl_ModelViewMatrixInverse,gl_LightSource[0].position, vLightDir);
   normalizeGlNormal(vNormal);
   
   //Calculates a set of spherical texture coordinates for sampling from a spherical environment map.
   sphereMap(vViewDir, vNormal, vReflectTexCoord);

   //Calculate the distance of the vertex from the view
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);

   //Finally, mix the results with the fog amount and compute the gl_Position
   computeVehicleFog(gl_Fog.end, vDistance, vFog);
   vec4 outGLPosTemp;
   setVehicleGLPosition(outGLPosTemp);
   gl_Position = outGLPosTemp;       
   
   //our position and normal is in local space and we want it it
   vPos = inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex;
   worldNormal = mat3(inverseViewMatrix) * gl_NormalMatrix * gl_Normal;   
}

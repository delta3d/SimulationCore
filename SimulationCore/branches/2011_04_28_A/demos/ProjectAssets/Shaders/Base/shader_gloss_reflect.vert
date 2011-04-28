uniform mat4 inverseViewMatrix;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vViewDir;
varying vec2 vReflectTexCoord;
varying float vFog;
varying float vDistance;
varying vec3 vPos;

void sphereMap(vec3, vec3, out vec2);
void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);

/**
 * This is the vertex shader to a simple set of shaders that calculates
 * per pixel lighting assuming one directional light source.  It also
 * calculates texture coordinates suitable for doing sphere map reflection.
 */
void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0] = gl_MultiTexCoord0;
   
   vPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;
   
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
      inverseViewMatrix[1].xyz,
      inverseViewMatrix[2].xyz);
   
   vNormal = normalize(inverseView3x3 * gl_NormalMatrix * gl_Normal);
   vLightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);
   
   vViewDir = normalize((gl_ModelViewMatrix * gl_Vertex).xyz * inverseView3x3);
   
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, vDistance);
   //Calculates a set of spherical texture coordinates for sampling from a spherical environment map.
   sphereMap(vViewDir, vNormal, vReflectTexCoord);
}

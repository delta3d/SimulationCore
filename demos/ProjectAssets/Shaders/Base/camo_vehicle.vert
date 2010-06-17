//uniform mat4 inverseViewMatrix;
uniform mat4 osg_ViewMatrixInverse;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying vec3 vPos;
varying vec4 vModelVertPos;
varying vec2 vDiffuseUVs;
varying vec2 vOverlayUVs; // For damage state marks.

// EXTERNAL FUNCTIONS
void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);

// camo_paint.vert
void calculateCamoAndDamageUVs();

// From: concealment_scale.vert
vec3 getConcealmentScaledVertex(vec3 vert);

void main()
{
   gl_Position = gl_ModelViewProjectionMatrix * vec4(getConcealmentScaledVertex(gl_Vertex.xyz), 1.0);
   
   gl_TexCoord[0]  = gl_MultiTexCoord0;
   
   // Compute the UVs and varyings for camo diffuse and damage overlay.
	calculateCamoAndDamageUVs();

   //moves the position, normal, and light direction into world space   
   //vPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;
   //mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);
   vPos = (osg_ViewMatrixInverse * gl_ModelViewMatrix * gl_Vertex).xyz;
   mat3 inverseView3x3 = mat3(osg_ViewMatrixInverse[0].xyz, osg_ViewMatrixInverse[1].xyz, osg_ViewMatrixInverse[2].xyz);

   vNormal = inverseView3x3 * gl_NormalMatrix * gl_Normal;

   vLightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);

   float distance;
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, distance);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, distance);  
}
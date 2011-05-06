uniform mat4 inverseViewMatrix;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vDistance;
varying vec3 vPos;

void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);


void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0]  = gl_MultiTexCoord0;

   //moves the position, normal, and light direction into world space   
   vPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);

   vNormal = inverseView3x3 * gl_NormalMatrix * gl_Normal;

   vLightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);

   calculateDistance(gl_ModelViewMatrix, gl_Vertex, vDistance);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, vDistance);  
}
uniform mat4 inverseViewMatrix;

const float UnderWaterViewDistance = 100.0;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vUnderWaterFog;
varying vec4 camPos;
varying vec3 vPos;

void calculateDistance(mat4, vec4, out float);
float computeFog(float, float, float);
float GetHeightOnWaterSuface(vec2);

void main()
{
   gl_Position = ftransform();
   gl_TexCoord[0]  = gl_MultiTexCoord0;

   //moves the position, normal, and light direction into world space   
   vPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;
   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);

   camPos = inverseViewMatrix[3];

   vNormal = inverseView3x3 * gl_NormalMatrix * gl_Normal;

   vLightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);

   float distance;
   calculateDistance(gl_ModelViewMatrix, gl_Vertex, distance);
   vFog = computeFog(gl_Fog.end * 0.15, gl_Fog.end, distance);  

   vUnderWaterFog = computeFog(1.0, UnderWaterViewDistance, distance); 
      
}

/**
*  This shader is applied to the water spray particles from the surface vessel actors.
*
*  @author Bradley Anderegg
*/

uniform mat4 inverseViewMatrix;

varying vec3 lightContrib;
varying vec4 vertexColor;
//varying float waterHeight;
varying vec3 worldPos;

float GetHeightOnWaterSuface(vec2 point);

void main()
{
   gl_TexCoord[0] = gl_MultiTexCoord0; 
   vertexColor = gl_Color;
   
   lightContrib = vec3(0.0, 0.0, 0.0);
   
   worldPos = (inverseViewMatrix * gl_ModelViewMatrix * gl_Vertex).xyz;

   //waterHeight = GetHeightOnWaterSuface(worldPos.xy);
   //worldPos.z = 0.25 + max(worldPos.z, waterHeight);

   gl_Position = gl_ModelViewProjectionMatrix * vec4(worldPos.xyz, 1.0);

   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
   inverseViewMatrix[1].xyz,
   inverseViewMatrix[2].xyz);
   
   vec3 lightDir = normalize(inverseView3x3 * gl_LightSource[0].position.xyz);   
   
   vec3 psuedo_normal = vec3(0.0, 0.0, 1.0);
   
   lightContrib += vec3(gl_LightSource[0].ambient);
   lightContrib += vec3(gl_LightSource[0].diffuse * max(0.0, dot(lightDir, psuedo_normal)));
   
   lightContrib = clamp(lightContrib, 0.0, 1.0);
   
}

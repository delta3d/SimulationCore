uniform vec3 sunPosition;
uniform float effectRadius;


void main(void)
{   
   vec4 ecPosition = gl_ModelViewMatrix * vec4(sunPosition.xyz, 1.0);   
   
   vec4 viewPosCenter = vec4(100.0 * normalize(ecPosition.xyz), 1.0);  
         
   vec4 viewPosVert = viewPosCenter;
   viewPosVert.xy += gl_Vertex.xy * 25.0;
   

   gl_Position = gl_ProjectionMatrix * viewPosVert;
   gl_TexCoord[0]  = gl_MultiTexCoord0;   
   
}

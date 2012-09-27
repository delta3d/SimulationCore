uniform vec3 sunPosition;
uniform float effectRadius;

uniform mat4 inverseViewMatrix;
uniform mat4 osg_ViewMatrixInverse; 


varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;
varying vec3 vPos;
varying vec4 vCenterPixelOfEffect;

void main(void)
{   
   vec4 ecPosition = gl_ModelViewMatrix * vec4(sunPosition.xyz, 1.0);   
   
   vViewPosCenter = vec4(100.0 * normalize(ecPosition.xyz), 1.0);  
         
   vViewPosVert = vViewPosCenter;
   //vViewPosVert.xy += gl_Vertex.xy * effectRadius * 100.0;
   vViewPosVert.xy += gl_Vertex.xy * 20.0;
   
   vPos = (osg_ViewMatrixInverse * vViewPosVert).xyz;
   vCenterPixelOfEffect = gl_ProjectionMatrix * vViewPosCenter;
   vCenterPixelOfEffect.xyz = vCenterPixelOfEffect.xyz / vCenterPixelOfEffect.w;

   gl_Position = gl_ProjectionMatrix * vViewPosVert;
   gl_TexCoord[0]  = gl_MultiTexCoord0;   
   
}

uniform sampler2D diffuseTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;


varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying vec3 vPos;



void main(void)
{
   //Computes the Diffuse Color
   vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);
   
   vec3 color = vec3(diffuseColor.r, diffuseColor.g, 1.0);
   
   gl_FragColor = vec4(mix(color, gl_Fog.color.rgb, vFog), 0.5);//diffuseColor.a);  
}


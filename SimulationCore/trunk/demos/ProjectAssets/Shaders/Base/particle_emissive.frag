
uniform sampler2D diffuseTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;

uniform float Intensity;

varying vec4 vertexColor;

void main(void)
{
   vec4 baseColor = texture2D(diffuseTexture, gl_TexCoord[0].st); 
   
   //intensify the red component if night vision is enabled
   float nvgContrib = NVG_Enable * Intensity;
   
   //add in the nvg components
   vec3 diffuseLight = vec3(diffuseRadiance, gl_LightSource[1].diffuse.g, gl_LightSource[1].diffuse.b);
   vec3 lightContrib = nvgContrib * (diffuseLight + vec3(ambientRadiance, gl_LightSource[1].ambient.g, gl_LightSource[1].ambient.b));
   
   baseColor.xyz += lightContrib;
   
   vec4 vertexColorContrib = vec4(vertexColor + NVG_Enable);

   gl_FragColor = baseColor * vertexColorContrib;
}



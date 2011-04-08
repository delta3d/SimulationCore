uniform sampler2D diffuseTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;
uniform float ColorOscillate;


varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying vec3 vPos;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3, vec3, out vec3);


void main(void)
{
   //Computes the Diffuse Color
   vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);
   
   //Compute the Light Contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);
   
   vec3 dynamicLightContrib;
   dynamic_light_fragment(vNormal, vPos, dynamicLightContrib);
   lightContrib += dynamicLightContrib + (dynamicLightContrib * (10.0 * NVG_Enable));
   
   //add in the nvg components
   vec3 diffuseLight = vec3(diffuseRadiance, gl_LightSource[1].diffuse.g, gl_LightSource[1].diffuse.b);
   lightContrib += NVG_Enable * diffuseLight + vec3(ambientRadiance, gl_LightSource[1].ambient.g, gl_LightSource[1].ambient.b);   
   lightContrib = clamp(lightContrib, 0.0, 1.0 + (10000.0 * NVG_Enable));


   //Compute the specular contribution
   //vec3 reflectVec = reflect(vLightDir, vNormal);
   //float reflectContrib = max(0.0,dot(reflectVec, -vViewDir));
   //vec3 specularContrib = 0.4 * (pow(reflectContrib, 16.0));   
   // don't apply specular greater than the light contrib or we get glowing trucks in the dark...
   //color += min(specularContrib, lightContrib);
   
   vec3 color = clamp(lightContrib * vec3(diffuseColor), 0.0, 1.0);
   vec3 redDiffuse = vec3(1.0, diffuseColor.g * 0.3, diffuseColor.b * 0.3); // shift color toward red.
   color = mix(color, redDiffuse, ColorOscillate); 
   
   
   gl_FragColor = vec4(mix(color, gl_Fog.color.rgb, vFog), diffuseColor.a);  
   //gl_FragColor = vec4(0.5, 1.0, 0.8, 1.0);
}


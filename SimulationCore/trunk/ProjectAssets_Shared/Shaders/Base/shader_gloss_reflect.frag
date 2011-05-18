uniform sampler2D reflectTexture;
uniform sampler2D glossTexture;
uniform sampler2D diffuseTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;
uniform bool writeLinearDepth;

varying vec3 vNormal;
varying vec3 vLightDir;
varying vec3 vViewDir;
varying vec2 vReflectTexCoord;
varying float vFog;
varying vec3 vPos;
varying float vDistance;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);
float computeFragDepth(float, float);

void main(void)
{
   float fragDepth = computeFragDepth(vDistance, gl_FragCoord.z);
   gl_FragDepth = fragDepth;

   //currently we only write a linear depth when doing a depth pre-pass
   //as an optimization we return immediately afterwards
   if(writeLinearDepth)
   {
      return;
   }
   
   vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);
   vec3 glossMap = vec3(texture2D(glossTexture,gl_TexCoord[0].st)); 
   vec3 reflectMap = vec3(texture2D(reflectTexture,vReflectTexCoord));

   //Compute the light contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);
   
   //accumulate the dynamic light contribution
   vec3 dynamicLightContrib;
   dynamic_light_fragment(vNormal, vPos, dynamicLightContrib);
   
   vec3 spotLightContrib;
   spot_light_fragment(vNormal, vPos, spotLightContrib);
   dynamicLightContrib += spotLightContrib;
   
   lightContrib += dynamicLightContrib + (dynamicLightContrib * (10.0 * NVG_Enable));
   
   //add in the nvg components
   vec3 diffuseLight = vec3(diffuseRadiance, gl_LightSource[1].diffuse.g, gl_LightSource[1].diffuse.b);
   lightContrib += NVG_Enable * diffuseLight + vec3(ambientRadiance, gl_LightSource[1].ambient.g, gl_LightSource[1].ambient.b);
   
   lightContrib = clamp(lightContrib, 0.0, 1.0 + (10000.0 * NVG_Enable));
   
   //Compute the specular contribution
   vec3 reflectVec = reflect(vLightDir, vNormal);
   float reflectContrib = max(0.0,dot(reflectVec, -vViewDir));
   vec3 specularContrib = vec3(glossMap.r) * (pow(reflectContrib, 16.0));
   
   vec3 color = mix(lightContrib * vec3(diffuseColor), reflectMap, min(lightContrib, glossMap.g));
   
   // don't apply specular greater than the light contrib or we get glowing trucks in the dark...
   color += min(specularContrib, lightContrib);						
   
   //add in the dynamic light reflective
   color = clamp(color, 0.0, 1.0);
   
   //gl_FragColor = vec4(vFogForXDistanceForY.xxx, 1.0);
   gl_FragColor = vec4(mix(color, gl_Fog.color.rgb, vFog), diffuseColor.a);  
}


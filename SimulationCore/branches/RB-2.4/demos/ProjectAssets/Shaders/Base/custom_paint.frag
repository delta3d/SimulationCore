uniform sampler2D diffuseTexture;
uniform sampler2D colorMaskTexture;

uniform vec4 color1;
uniform vec4 color2;
uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;


varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying vec3 vPos;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);


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

   vec3 spotLightContrib;
   spot_light_fragment(vNormal, vPos, spotLightContrib);
   dynamicLightContrib += spotLightContrib;
   
   //add in the nvg components
   vec3 diffuseLight = vec3(diffuseRadiance, gl_LightSource[1].diffuse.g, gl_LightSource[1].diffuse.b);
   lightContrib += NVG_Enable * diffuseLight + vec3(ambientRadiance, gl_LightSource[1].ambient.g, gl_LightSource[1].ambient.b);
   
   vec4 colorMask = texture2D(colorMaskTexture, gl_TexCoord[0].st);
   
   lightContrib += colorMask.www;
   lightContrib = clamp(lightContrib, 0.0, 1.0 + (10000.0 * NVG_Enable));
   
   vec3 fadeFactor = max(diffuseColor.xyz, colorMask.www);
   vec3 color = lightContrib * (colorMask.xxx * color1.xyz * fadeFactor
		+ colorMask.yyy * color2.xyz * fadeFactor
		+ (vec3(1.0,1.0,1.0) - colorMask.xxx - colorMask.yyy) * fadeFactor);
//   lightContrib += colorMask.www;
   color = clamp(color, vec3(0.0,0.0,0.0), vec3(1.0,1.0,1.0));
   
//   color = clamp(lightContrib * color, 0.0, 1.0);
   
   gl_FragColor = vec4(mix(color, gl_Fog.color.rgb, vFog), diffuseColor.a);  
}


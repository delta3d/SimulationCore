uniform sampler2D diffuseTexture;

uniform vec4 color1;
uniform vec4 color2;
uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;
uniform bool writeLinearDepth;

varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vDistance;
varying vec3 vPos;
varying vec4 vModelVertPos;
varying vec2 vDiffuseUVs;
varying vec2 vOverlayUVs;
varying float vOverlayMult;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3, vec3, out vec3);
void spot_light_fragment(vec3, vec3, out vec3);
float computeFragDepth(float, float);


// EXTERNAL FUNCTIONS
// camo_paint.frag
vec4 getCamoAndDamageColor();

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


   //Computes the Diffuse Color
   vec4 diffuseColor = getCamoAndDamageColor();
   
   //Compute the Light Contribution
   vec3 lightContrib;
   lightContribution(vNormal, vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContrib);
   
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
   
   vec3 color = clamp(lightContrib * vec3(diffuseColor), 0.0, 1.0);
   
   gl_FragColor = vec4(mix(color, gl_Fog.color.rgb, vFog), diffuseColor.a);
}


uniform sampler2D diffuseTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform float NVG_Enable;
uniform float WaterHeight;// = 443.0;
uniform vec4 WaterColor;


varying vec3 vNormal;
varying vec3 vLightDir;
varying float vFog;
varying float vUnderWaterFog;
varying vec4 camPos;
varying vec3 vPos;

void lightContribution(vec3, vec3, vec3, vec3, out vec3);
void alphaMix(vec3, vec3, float, float, out vec4);
void dynamic_light_fragment(vec3, vec3, out vec3);
vec3 GetWaterColorAtDepth(float);
float GetHeightOnWaterSuface(vec2);
void spot_light_fragment(vec3, vec3, out vec3);

vec3 deep = 0.74 * WaterColor.xyz;  


void main(void)
{
   //Computes the Diffuse Color
   vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);
   
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
   
   vec3 lightContribUnderWaterFog;
   lightContribution(vec3(0.0, 0.0, 1.0), vLightDir, vec3(gl_LightSource[0].diffuse), vec3(gl_LightSource[0].ambient), lightContribUnderWaterFog);
   
   vec3 underWaterColor = lightContribUnderWaterFog * GetWaterColorAtDepth(camPos.z);
   
   float height = GetHeightOnWaterSuface(vPos.xy);
   float depth = height - vPos.z;
   depth = 0.5 + 10.0 * clamp(depth, -0.05, 0.05);

   color = mix( mix(color, gl_Fog.color.rgb, vFog), mix(color, underWaterColor, vUnderWaterFog), depth);

   gl_FragColor = vec4(color, diffuseColor.a);  
}


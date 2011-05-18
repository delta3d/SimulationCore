uniform sampler2D glowTexture;

uniform float diffuseRadiance;
uniform float ambientRadiance;
uniform vec4 lineCenterColor;
uniform vec4 lineGlowColor;
uniform float NVG_Enable;
uniform float Intensity;
uniform bool writeLinearDepth;

varying float vDistance;
varying vec4 vColor;
varying vec4 vTex0;
varying vec4 vTex1;

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
    vec4 texelColor0 = texture2D( glowTexture, vTex0.xy );
    vec4 texelColor1 = texture2D( glowTexture, vTex1.xy );
    vec4 diffuseColor = mix( texelColor0, texelColor1, vColor );
    
    // Modulate the color.
    float diff = diffuseColor.x - diffuseColor.y;
    diffuseColor.xyz = mix(lineCenterColor.xyz, lineGlowColor.xyz * diffuseColor.x, diff);

   //intensify the red component if night vision is enabled
   float nvgContrib = NVG_Enable * Intensity;
   
   //add in the nvg components
   vec3 diffuseLight = vec3(diffuseRadiance, gl_LightSource[1].diffuse.g, gl_LightSource[1].diffuse.b);
   vec3 lightContrib = nvgContrib * (diffuseLight + vec3(ambientRadiance, gl_LightSource[1].ambient.g, gl_LightSource[1].ambient.b));

   gl_FragColor = diffuseColor + vec4(lightContrib.xyz, 0.0);
   
}

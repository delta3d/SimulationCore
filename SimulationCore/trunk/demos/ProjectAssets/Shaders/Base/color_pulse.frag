uniform sampler2D diffuseTexture;
uniform sampler2D pulsePathTexture;
uniform float pulseOffset;
uniform vec4 pulseColor;
uniform bool writeLinearDepth;

varying float vDistance;

const float BLIP_WIDTH = 0.5;

float computeFragDepth(float, float);

void main (void)
{
   float fragDepth = computeFragDepth(vDistance, gl_FragCoord.z);
   gl_FragDepth = fragDepth;

   //currently we only write a linear depth when doing a depth pre-pass
   //as an optimization we return immediately afterwards
   if(writeLinearDepth)
   {
      return;
   }

	 vec4 pathFrag = texture2D(pulsePathTexture, gl_TexCoord[0].st);
    vec4 color = texture2D(diffuseTexture, gl_TexCoord[0].st);
    float temp = min(abs(pathFrag.z - pulseOffset), BLIP_WIDTH);
    pathFrag.z = temp * 10.0;
    color.xyz += (pulseColor.xyz * (1.0 - clamp(pathFrag.z,0.0,1.0)));
    color.xyz = clamp(color.xyz, vec3(0.0,0.0,0.0), vec3(1.0,1.0,1.0));
    gl_FragColor = color;
}

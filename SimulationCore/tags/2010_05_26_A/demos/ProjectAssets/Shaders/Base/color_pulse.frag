uniform sampler2D diffuseTexture;
uniform sampler2D pulsePathTexture;
uniform float pulseOffset;
uniform vec4 pulseColor;

const float BLIP_WIDTH = 0.5;

void main (void)
{
	 vec4 pathFrag = texture2D(pulsePathTexture, gl_TexCoord[0].st);
    vec4 color = texture2D(diffuseTexture, gl_TexCoord[0].st);
    float temp = min(abs(pathFrag.z - pulseOffset), BLIP_WIDTH);
    pathFrag.z = temp * 10.0;
    color.xyz += (pulseColor.xyz * (1.0 - clamp(pathFrag.z,0.0,1.0)));
    color.xyz = clamp(color.xyz, vec3(0.0,0.0,0.0), vec3(1.0,1.0,1.0));
    gl_FragColor = color;
}

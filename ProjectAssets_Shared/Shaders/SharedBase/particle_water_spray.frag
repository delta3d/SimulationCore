/**
*  This shader is applied to the water spray particles from the surface vessel actors.
*
*  @author Bradley Anderegg
*/

uniform sampler2D diffuseTexture;
uniform float WaterHeight;


varying vec3 lightContrib;
varying vec4 vertexColor;
//varying float waterHeight;
varying vec3 worldPos;

void main(void)
{
   vec4 baseColor = texture2D(diffuseTexture, gl_TexCoord[0].st); 
   
   //this uses the length of the light color to avoid the water turning red in the sunset
   baseColor.xyz *= length(lightContrib);
   gl_FragColor = baseColor * vertexColor;
   gl_FragColor.a = baseColor.a;
   
   //this makes particles below the water surface disappear
   //gl_FragColor.a = min(max(0.0, worldPos.z - WaterHeight), baseColor.a);
   if (worldPos.z - WaterHeight < 0.0)
      discard;
}

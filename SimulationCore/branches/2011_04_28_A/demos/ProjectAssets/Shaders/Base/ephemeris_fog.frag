// This Shader is dependent on the following files
//   ephemeris_fragment_functions.glsl
//   fragment_functions.glsl
// Please be sure to list any extra files that this shader becomes dependent on.

//Any distance below the completeFogDistance and the Sphere will be fog out completally.
//Any distance above the normalFogDistance and the Sphere will use the default distanceFogStart and distanceFogEnd.
//Any value inbetween will contribute some additional percentage of fog to the sphere
//Note - NormalFogDistance should never be set to 0.0 as it is used as a denominator in fragment calculations 
uniform float DistanceForHorizonFog; // normalFogDistance
uniform float DistanceForCompleteFog; // completeFogDistance

varying float vFog;
varying float vSunContribution;
varying vec4 lightColor;
 
void computeEphemerisDiffuse(out vec3 );
void computeSunSpecular(float, out float);
void computeBaseEphemerisColor(float, vec3, out vec3);
void computeFinalEphemerisColor(float, vec3, inout vec3);
void toggleSun(float, out float);
void alphaMix(vec3, vec3, float, float, out vec4);

void main(void)
{
   //Grabs different values from the Ephemeris Light Source so they can be used in later calculations
   vec3 diffuse;
   computeEphemerisDiffuse(diffuse);

   // make a specular spot for the sun's glow
   float sunSpecular;
   computeSunSpecular(vSunContribution, sunSpecular);
   
   //Compute the color of the Ephemeris a toned down base color based 
   vec3 color;
   computeBaseEphemerisColor(vSunContribution, diffuse, color);
   computeFinalEphemerisColor(sunSpecular, diffuse, color);
   	
   //Calculation that is used to determine if the Sun Light should shine through the fog or if 
   //the fog is too thick for the sun to shine through. AKA the Sun will only effect the fog
   //when the Visibile Distance is greater than the normalFogDistance (defined in the Vertex Shader)    
   float sunToggle;
   toggleSun(DistanceForHorizonFog, sunToggle);
   
   //Uses a mix to determine if the sun has any effect on the fog or not
   alphaMix(gl_Fog.color.rgb, color, sunToggle, vFog, gl_FragColor);
   gl_FragColor = max(gl_FragColor, vec4(gl_Fog.color.rgb, vFog));
}




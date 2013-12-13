// This Shader is dependent on the following files
//   ephemeris_vertex_functions.glsl
// Please be sure to list any extra files that this shader becomes dependent on.

//Any distance below the completeFogDistance and the Sphere will be fog out completally.
//Any distance above the normalFogDistance and the Sphere will use the default distanceFogStart and distanceFogEnd.
//Any value inbetween will contribute some additional percentage of fog to the sphere
//Note - NormalFogDistance should never be set to 0.0 as it is used as a denominator in fragment calculations 
//uniform float DistanceForHorizonFog; //const float normalFogDistance = 4000.0;
//uniform float DistanceForCompleteFog; //const float completeFogDistance = 3000.0;

varying float vFog;
varying float vSunContribution; 
varying vec4 lightColor;

//Normal Fog Start and End Points measured in degrees
// Start is the angle at which the sky fog band STARTS fogging in
// End is the angel at which the sky fog band is COMPLETELY fogged
const float distanceFogStart = 15.0;
const float distanceFogEnd = 4.0;

const float degreesToSkyTop = 90.0;

void computeSunContribution(out float);
void computeEphemerisFogEnd(float, float, out float);
void computeEphemerisFogStart(float, float, out float);
void computeEphemerisFog(float, float, float, out float);

void main()
{  
   //Compute the maximum amount of the suns light contribution on the Sphere from the Ephemeris Sun
   computeSunContribution(vSunContribution);

   //Computes a new Fog End point on the Ephmeris Sphere
   float fogEnd;
   computeEphemerisFogEnd(distanceFogEnd, degreesToSkyTop, fogEnd);
   
   //Computes a new Fog Start point on the Ephmeris Sphere
   float fogStart;
   computeEphemerisFogStart(distanceFogStart, fogEnd, fogStart);
   
   //Computes the total fog contribution at a given vertex
   computeEphemerisFog(degreesToSkyTop, fogStart, fogEnd, vFog);

   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
   lightColor = gl_Color;
}

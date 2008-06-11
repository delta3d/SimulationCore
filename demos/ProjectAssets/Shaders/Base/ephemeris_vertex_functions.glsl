// The following shaders make use of it least one of this functions:
//   ephemeris_fog_vert.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes


/////////////////////////////////////////////////////////////////////
/////////////////Start Ephemeris Specific Functions//////////////////
/////////////////////////////////////////////////////////////////////

uniform float DistanceForHorizonFog; 
uniform float DistanceForCompleteFog;

void computeSunContribution(out float sunContribution)
{
   vec3 lightDirection = normalize(vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position));
   sunContribution = max(0.0, dot(lightDirection, gl_Normal));
}


void computeEphemerisFogEnd(float distanceFogEnd, float degreesToSkyTop, out float fogEnd)
{
   //float fogEnd = clamp (distanceFogEnd + clamp( (1.0/(completeFogDistance-normalFogDistance )) * gl_Fog.end + -(1.0/(completeFogDistance-normalFogDistance )) * normalFogDistance , 0.0, 1.0) * 90.0, 0.0, 90.0);   
   float fogScale = (1.0/(DistanceForCompleteFog-DistanceForHorizonFog ));  
   fogEnd = clamp( (gl_Fog.end - DistanceForHorizonFog) * fogScale, 0.0, 1.0);   
   fogEnd = clamp(distanceFogEnd + (fogEnd * degreesToSkyTop), distanceFogEnd, degreesToSkyTop);
}

void computeEphemerisFogStart(float distanceFogStart, float fogEnd, out float fogStart)
{
   //Note - 95.0 is used because an overlap is required to fog out the "top" of the sphere
   fogStart = clamp (distanceFogStart + fogEnd, 0.0, 95.0);
}

void computeEphemerisFog(float degreesToSkyTop, float fogStart, float fogEnd, out float vFog)
{   
   //Calculates the Fog Start and Fog End points from degrees to radians so they can be used to calculate a Fog Value
   float fogAngle = radians(degreesToSkyTop - fogStart);
   float fogEndInRadians = radians(degreesToSkyTop - fogEnd);

   //Determine the total fog contribution - This is a band effect where the backdrop fog is
   // gradiated from the zenith (top) of the sphere down to the horizon in a linear fashion using 
   // the fogEnd and fogStart angles computed above
   float fogAngleScale = (1.0/(fogEndInRadians-fogAngle));
   float angleFromTopToVertex = acos(dot(vec3(0.0, 0.0, 1.0), gl_Normal)); // in radians
   vFog = fogAngleScale * (angleFromTopToVertex - fogAngle);
}

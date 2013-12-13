// The following shaders make use of it least one of this functions:
//   ephemeris_fog_frag.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

 
/////////////////////////////////////////////////////////////////////
//////////////////Start Ephmeris Specific Functions//////////////////
/////////////////////////////////////////////////////////////////////

void computeEphemerisDiffuse(out vec3 diffuse)
{
   diffuse = gl_LightSource[0].diffuse.rgb;
   diffuse = diffuse + vec3(gl_LightSource[0].ambient);
}

void computeSunSpecular(float sunContribution, out float sunSpecular)
{
   // make a specular spot for the sun's glow - this clamp seems redundant, but fixes an 
   // oddity on some GPUs where the pow creates odd lines when the specular approaches zero.
   sunSpecular = clamp(pow(sunContribution, 25.0), 0.0, 1.0);
}

void computeBaseEphemerisColor(float sunContribution, vec3 diffuse, out vec3 color)
{
   color = 0.5 * (sunContribution * diffuse.rgb);
}

void computeFinalEphemerisColor(float sunSpecular, vec3 diffuse, inout vec3 color)
{
   //Weird Balance of Colors that keeps the cool sunset / sunrise effects without overwhelming the sky
   color += 0.5 * (sunSpecular * diffuse.rgb) + (gl_Fog.color.rgb * 0.5);
}

void toggleSun(float distanceForHorizonFog, out float sunToggle)
{
   sunToggle = clamp( (gl_Fog.end / distanceForHorizonFog) - 1.0 , -1.0, 1.0);
   sunToggle = clamp ( 1.0 / sunToggle, 0.0, 1.0);
}

// The following shaders make use of it least one of this functions:
//   terrain_vertex.glsl

// Before making changes to the following functions please be sure to check all
// the other shaders that use them to make sure that they will not be adversly 
// affected by your changes. Also, if you create another shader that uses any of 
// the functions please add that shader's file name to the list above. Thanks :)
// -Matthew "w00by" Stokes

/////////////////////////////////////////////////////////////////////
//////////////Start Terrain Specific Vertex Functions////////////////
/////////////////////////////////////////////////////////////////////

void computeShaderWeight(vec3 normal, out float shaderMultiplier)
{
   shaderMultiplier = abs(normal.z);
}

void computeWeights(float distance, float maxDistance, float shaderMultiplier, out vec3 weights)
{
   weights[0] = (1.0 - clamp(distance/maxDistance,0.0,1.0)) * shaderMultiplier;
   float blurSquared = weights[0] * weights[0];
   weights[1] = clamp(blurSquared * 1.3, 0.0, 1.0); // more affect closer up.
   weights[2] = sin(weights[0]) * blurSquared; // fine only effects when VERY close.
} 

void setTerrainGLPosition(out vec4 position)
{
   position = ftransform();
}

void computeTerrainFog(float endFog, float fogDistance, out float fog)
{
   // Note - the best equation is: 2 ^ (-8 * dist * dist) where dist = 0 at fog_start and 1 at fog_end. 
   // This gives a nice S sort of curve with fog = 1 at x = 0 and fog = 0 at X = 1.
   //float fogXValue = clamp((vDistance - fog_start) * (1/(fog_end - fog_start)), 0.0, 1.0); 
   //fog = pow(2.0, -8 * fogXValue * fogXValue);
   
   // Here, we just use linear.
   float startFog = endFog * 0.20;
   fog = clamp((fogDistance - startFog) / (endFog - startFog), 0.0, 1.0);
}

void computeTerrainGLTexCoord(out vec4 texCoord0)
{
   texCoord0 = gl_MultiTexCoord0;
}

void computGLFogFragCoord(out float fogFragCoord)
{
   fogFragCoord = gl_FogCoord;
}

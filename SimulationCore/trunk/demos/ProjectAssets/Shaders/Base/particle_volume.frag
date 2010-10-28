uniform vec4 volumeParticleColor;
uniform sampler2D depthTexture;
uniform sampler3D noiseTexture;
uniform float osg_SimulationTime;
uniform mat4 inverseViewMatrix;

varying vec4 vPos;
varying vec4 vOffset;
varying vec3 vRadius;
varying mat3 vNormalMatrix;


float computeFog(float startFog, float endFog, float fogDistance)
{
   float fogTemp = pow(2.0, (fogDistance - startFog) / (endFog - startFog)) - 1.0;
   return clamp(fogTemp, 0.0, 1.0);
}

void main(void)
{

   vec3 normal;
   normal.xy = vRadius.xy * 2.0 - vec2(1.0, 1.0);
   float r = dot(normal.xy, normal.xy);
   normal.z = -sqrt(1.0 - r);
   normal = vec3(normal.x, normal.z, normal.y);

   if(r > 1.0) discard;

   float sceneDepth = texture2D(depthTexture, gl_FragCoord.xy).r;

   vec3 noiseCoords =  normal + (vOffset.xyz) + vec3(0.0, 0.0, 0.25 * osg_SimulationTime);
   float noise  = abs( (texture3D(noiseTexture, noiseCoords)).a );
   
    if(noise < r)
        discard;
    else
        noise -= r;

   vec4 pixelPos = vec4(vPos.xyz + (normal * vec3(vRadius.z)), 1.0);
   vec4 clipSpacePos = gl_ModelViewProjectionMatrix * pixelPos;
   float depth = abs(clipSpacePos.z / clipSpacePos.w);

   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
   inverseViewMatrix[1].xyz,
   inverseViewMatrix[2].xyz);

   normal = inverseView3x3 * vNormalMatrix * normal;

   float fogDist = abs(sceneDepth - depth);//, 0.0, 1.0);
   float fogAmt = computeFog(0.0, 0.1, 10.0 * fogDist);
   gl_FragColor = vec4(volumeParticleColor.xyz, fogAmt * noise * noise);
   //gl_FragDepth = depth;
}


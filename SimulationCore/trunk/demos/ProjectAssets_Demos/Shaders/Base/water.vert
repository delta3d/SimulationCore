//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
//The wave parameters are packed into two vec4's like so
// [Length, Speed, Amplitude, Frequency], [Q, reserved for later use, Direction.x, Direction.y]
//////////////////////////////////////////////////////////////////////////////////////////////////
const int WAVE_OFFSET = 0;
const int NUMWAVES = 8;
const int MAX_WAVES = 8;
uniform vec4 waveArray[2 * MAX_WAVES];
uniform float waterPlaneFOV;
uniform float WaterHeight;// = 443.0;
const float UnderWaterViewDistance = 50.0;

 
varying vec4 pos;
varying vec3 lightVector;
varying float distanceScale;
varying vec2 vFog;
varying vec2 vertexWaveDir;
varying vec3 shaderVertexNormal;

uniform float elapsedTime;
uniform float maxComputedDistance;
uniform mat4 inverseViewMatrix;
uniform float modForFOV;
uniform float waveDirection;
uniform vec3 cameraHPR;
uniform vec3 cameraRecenter;

float computeFog(float, float, float);

void main(void)
{   
   float distBetweenVertsScalar;
   vec4 camPos = inverseViewMatrix[3];
 

   // This scalar stretches the verts out, so we don't waste 50,000 verts directly under us.
   // As we move up, it pushes the verts out.
   float cameraHeight = max(0.1, abs(camPos.z - WaterHeight));
   float scalar = min(15.0, log(cameraHeight/20.0 + 1.0)) + min(10.0, max(0.0, (cameraHeight-10.0))/50.0);
   scalar = max(0.1, scalar);

   camPos.z = 0.0;
   camPos.w = 0.0;
   
   float cameraOffset = 90.0 + cameraHPR.x;
   float radialScaleDegrees = cameraOffset + (gl_Vertex.x * waterPlaneFOV);
   float posX = gl_Vertex.y * cos(radians(radialScaleDegrees));
   float posY = gl_Vertex.y * sin(radians(radialScaleDegrees));
   vec4 localVert = vec4(posX, posY, 0.0, gl_Vertex.w);

   localVert = vec4(localVert.x * scalar, localVert.y * scalar, 0.0, 1.0);

   // Compute a scalar based on the verts proximity to the clip plane. As it approaches
   // the clip plane, we don't want to adjust the height (so it's sort of flat at the horizon).
   float distance = length(localVert.xy);
   float maxDistance = 2000.0;

   distanceScale = (1.0 - clamp(distance / (maxDistance * modForFOV), 0.0, 1.0));   
   float distFromCamera = distance;
   float distBetweenVerts = gl_Vertex.z;
   distBetweenVertsScalar = gl_Vertex.z * scalar * 3.5;// / modForFOV;
 
   pos = camPos + localVert;   
   pos.z = WaterHeight;
   vec2 offsetPos = pos.xy - cameraRecenter.xy;

   float zModifier = 0.0;
   vertexWaveDir = vec2(0.0);

   shaderVertexNormal = vec3(0.0, 0.0, 1.0);
   // There are 2 vec4's of data per wave, so the loop is MAX_WAVES * 2 but increments by 2's
   for(int i = 2 * WAVE_OFFSET; i < NUMWAVES * 2; i+=2)
   {           
      float waveLen = waveArray[i].x;
      float speed = waveArray[i].y;
      float freq = waveArray[i].w;
      float amp = waveArray[i].z;
      //float Q = 0.001;//waveArray[i+1].x;
      vec2 waveDir = waveArray[i + 1].zw;
      
      // scale out small waves as we get too far away
      amp *= 1.0 - clamp((distBetweenVertsScalar) / (waveLen), 0.0, 0.999);

      float mPlusPhi =  (freq * (speed * elapsedTime + offsetPos.x * waveDir.x + waveDir.y * offsetPos.y)); 
      
      float k = max(waveArray[i+1].x, 4.00001);

      float sinPhi = sin(mPlusPhi);
      //cos/sin of the sum of the previous two variables
      float sinDir = pow((sinPhi + 1.0) / 2.0, k);

      zModifier += 0.000001 + amp * sinDir;

      float vertexDerivativeScalar = freq * amp * pow((sinPhi + 1.0) * 0.5, k - 1.0) * cos(mPlusPhi);
      shaderVertexNormal.x += k * waveDir.x * vertexDerivativeScalar;
      shaderVertexNormal.y += k * waveDir.y * vertexDerivativeScalar;      
      vertexWaveDir += waveDir; 
   }

   vertexWaveDir = normalize(vertexWaveDir);

   shaderVertexNormal.x = -shaderVertexNormal.x;
   shaderVertexNormal.y = -shaderVertexNormal.y;
   shaderVertexNormal = normalize(shaderVertexNormal);

   //we scale out the waves based on distance to keep the water from going through the terrain
   float heightScalar = 1.0 - min(1.0, max(0.0001, (distance - 100.0) / 200.0));
   pos.z = WaterHeight + (zModifier * heightScalar);

   //transform our vector into screen space
   mat4 mvp = gl_ModelViewProjectionMatrix;
   //mvp[3] = vec4(0.0, 0.0, 0.0, 1.0);
   gl_Position = mvp * pos;
   
   float fog_distance = length(pos - inverseViewMatrix[3]);
   pos.xy = localVert.xy; // used to allow more precision in the frag shader.

   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
       inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);
   
   //very far off in worldspace
   lightVector = (inverseView3x3 * gl_LightSource[0].position.xyz);
   //compute fog color for above water and under water
   vFog.x = computeFog(gl_Fog.end * 0.15, gl_Fog.end, fog_distance);  
   vFog.y = computeFog(1.0, UnderWaterViewDistance, fog_distance);

}

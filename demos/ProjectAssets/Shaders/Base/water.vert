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
varying vec4 viewDir;
varying vec3 vertexNormal;
//varying vec4 camPos;
varying vec3 lightVector;
varying float distanceScale;
varying float distBetweenVertsScalar;
varying vec2 vFog;
varying float ampScale[MAX_WAVES];

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

   localVert = vec4(localVert.x * scalar, localVert.y * scalar, 0.0, localVert.w);

   // Compute a scalar based on the verts proximity to the clip plane. As it approaches
   // the clip plane, we don't want to adjust the height (so it's sort of flat at the horizon).
   float distance = length(localVert.xy);
   float maxDistance = 2000.0;

   distanceScale = (1.0 - clamp(distance / (maxDistance * modForFOV), 0.0, 1.0));   
   float distFromCamera = distance;
   float distBetweenVerts = gl_Vertex.z;
   distBetweenVertsScalar = gl_Vertex.z * scalar * 1.0 / modForFOV;
 
   pos = camPos + localVert;   
   pos.z = WaterHeight;
   vec2 offsetPos = pos.xy - cameraRecenter.xy;

   vertexNormal = vec3(0.0, 0.0, 0.0);

   //vec2 prevPos = pos.xy;
   float zModifier = 0.0;

   // There are 2 vec4's of data per wave, so the loop is MAX_WAVES * 2 but increments by 2's
   for(int i = WAVE_OFFSET; i < NUMWAVES * 2 + WAVE_OFFSET; i+=2)
   {           
      float waveLen = waveArray[i].x;
      float speed = waveArray[i].y;
      float freq = waveArray[i].w;
      float amp = waveArray[i].z;
      //float Q = 0.001;//waveArray[i+1].x;
      vec2 waveDir = waveArray[i + 1].zw;
      
      // scale out small waves as we get too far away
      //float wavePulseScalar = /*waveLen */ 5.0 / 25.0;
      //float wavePulseFactor = 1.0 - clamp(1.25 * sin(dot(waveDir.yx, pos.xy) * wavePulseScalar), 0.0, 1.0);
      amp *= 1.0 - clamp((distBetweenVertsScalar) / (waveLen), 0.0, 0.999);
      //amp = amp * wavePulseFactor;

      //speed * freq * time
      //float phi = speed * freq * elapsedTime;
      //freq * waveDir DOT vertexm
      //float m = dot(freq * waveDir, prevPos);
      //float mPlusPhi = m + phi;
      float mPlusPhi =  (freq * (speed * elapsedTime + offsetPos.x * waveDir.x + waveDir.y * offsetPos.y)); 
      
      float k = max(waveArray[i+1].x, 1.00001);

      float sinPhi = sin(mPlusPhi);
      //cos/sin of the sum of the previous two variables
      float sinDir = pow((sinPhi + 1.0) / 2.0, k);

      zModifier += 0.000001 + amp * sinDir;
     
      float vertexDerivativeScalar = freq * amp * pow((sinPhi + 1.0) * 0.5, k - 1.0) * cos(mPlusPhi);
      vertexNormal.x += k * waveDir.x * vertexDerivativeScalar;
      vertexNormal.y += k * waveDir.y * vertexDerivativeScalar;
   }

   pos.z = WaterHeight + zModifier;

   float heightScalar = 1.0 - min(1.0, max(0.1, (distance - 100.0) / 200.0));
   pos.z *= heightScalar;

   vertexNormal.x = -vertexNormal.x;
   vertexNormal.y = -vertexNormal.y;
   vertexNormal.z = 1.0;
   vertexNormal = normalize(vertexNormal);

   //transform our vector into screen space
   gl_Position = gl_ModelViewProjectionMatrix * pos;
   
   //camPos = inverseViewMatrix[3];
   viewDir = (pos - inverseViewMatrix[3]);
   float fog_distance = length(viewDir);
   viewDir /= fog_distance;
   pos.xy = localVert.xy; // used to allow more precision in the frag shader.

   mat3 inverseView3x3 = mat3(inverseViewMatrix[0].xyz, 
       inverseViewMatrix[1].xyz, inverseViewMatrix[2].xyz);
   
   //very far off in worldspace
   lightVector = (inverseView3x3 * gl_LightSource[0].position.xyz);
   //compute fog color for above water and under water
   vFog.x = computeFog(gl_Fog.end * 0.15, gl_Fog.end, fog_distance);  
   vFog.y = computeFog(1.0, UnderWaterViewDistance, fog_distance);

}

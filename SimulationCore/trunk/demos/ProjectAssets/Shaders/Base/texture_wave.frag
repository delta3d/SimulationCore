//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
//The wave parameters are packed into two vec4's like so
// [Length, Speed, Steepness, Frequency]
//////////////////////////////////////////////////////////////////////////////////////////////////
const int MAX_WAVES = 32;
uniform vec4 TextureWaveArray[MAX_WAVES];

uniform sampler2D reflectionMap;
uniform float elapsedTime;
uniform mat4 inverseViewMatrix;
uniform float textureWaveChopModifier;

const int CURWAVE=0;
const int numWaves = 32;//16;
const float twoPI = 6.283185;


void main (void)
{  
   vec3 camPos = inverseViewMatrix[3].xyz;
                                    
   
   float resolutionScalar = 1.0 + clamp(floor(sqrt(camPos.z) / 5.0), 0.0, 8.0); 
   float ampOverLength = 1.0 / (256.0 * resolutionScalar);

   vec3 textureNormal = vec3(0.0, 0.0, 0.0);  
   for(int i = 0+CURWAVE; i < numWaves+CURWAVE; ++i)
   {   
      float dir = pow(-1.0, float(i)) * float(i) * (textureWaveChopModifier / numWaves);
      float dirAsRad = radians(dir);//radians(waveDirArray[i]);
      //float dirAsRad = radians(waveDirArray[i]);
      float dirCos = cos(dirAsRad);
      float dirSin = sin(dirAsRad);
      vec2 waveDir = vec2(dirSin, dirCos);
      
      float waveLength = TextureWaveArray[i].x;
      float freq = TextureWaveArray[i].w;
      float amp = waveLength * ampOverLength;
      float steepness = TextureWaveArray[i].z;
      float speed = TextureWaveArray[i].y;   
   
      //speed * freq * time   
      float phi = 0.5 * speed * freq * elapsedTime;
      
      vec2 resolution = gl_FragCoord.xy / (64.0 * resolutionScalar);
      //float twoLSqrd = pow(2.0 * waveLength, 2.0);
      //vec2 tilingSize = sqrt(twoLSqrd / dot(waveDir, waveDir));
      //tilingSize *= (1.0 + int(1.0 / waveLength));
           
      //freq * waveDir DOT vertex
      //float m = dot( freq * waveDir, resolution * tilingSize);      
      float m = dot( freq * waveDir, resolution);                

      float k = 1.1 * steepness;
      float vertexDerivativeScalar = freq * amp * pow((sin(m + phi) + 1.0) * 0.5, k - 1.0) * cos(m + phi);

      textureNormal.x += k * waveDir.x * vertexDerivativeScalar;
      textureNormal.y += k * waveDir.y * vertexDerivativeScalar;

   }

   textureNormal.x = -textureNormal.x;
   textureNormal.y = -textureNormal.y;
   textureNormal.z = 1.0;
   
   textureNormal = normalize(textureNormal); 

   //scale it into color space
   textureNormal += 1.0;
   textureNormal /= 2.0;
    
   gl_FragColor = vec4(textureNormal, 0.0);
   //gl_FragColor = vec4(waveDirArray[i].x, waveDirArray[i].y, 0.0, 1.0);
}

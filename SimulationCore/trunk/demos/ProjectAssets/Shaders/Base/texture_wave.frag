//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////
//The wave parameters are packed into two vec4's like so
// [Length, Speed, Steepness, Frequency]
//////////////////////////////////////////////////////////////////////////////////////////////////
uniform sampler2D reflectionMap;
uniform float elapsedTime;
uniform mat4 inverseViewMatrix;
uniform float textureWaveChopModifier;
uniform float WaterHeight;

const int CURWAVE=0;
const int numWaves = 32;//16;
const float twoPI = 6.283185;


const float kArray[] = {1.33, 1.76, 3.0, 2.246,
                              1.0, 3.71, 1.0, 1.75,
                              1.5, 1.0, 1.0, 2.0,
                              2.2, 2.0, 1.113, 1.0,
                              1.33, 1.76, 3.0, 2.246,
                              1.0, 3.71, 1.0, 1.75,
                              1.5, 1.0, 1.0, 2.0,
                              2.2, 2.0, 1.113, 1.0};
                  
const float waveLengthArray[] = {0.1788, 0.0535, 0.12186, 0.24,
                                 0.14, 0.116844, 0.97437, 0.0805,
                                 0.067, 0.3565, 0.67135 , 0.191,
                                 0.155, 0.13917, 0.275, .448,
                                 0.1788, 0.0535, 0.12186, 0.24,
                                 0.14, 0.116844, 0.97437, 0.0805,
                                 0.067, 0.3565, 0.67135 , 0.191,
                                 0.155, 0.13917, 0.275, .448};
                                  
const float waveSpeedArray[] = {0.0953, 0.03839, 0.0311, 0.04221,
                                0.11497, 0.143213, 0.14571, 0.051181,
                                
                                0.01473, 0.1531, 0.2131, 0.0221,
                                0.121497, 0.1213, 0.14571, 0.1181,
                                0.0953, 0.03839, 0.0311, 0.04221,
                                0.11497, 0.143213, 0.14571, 0.051181,
                                
                                0.01473, 0.1531, 0.2131, 0.0221,
                                0.121497, 0.1213, 0.14571, 0.1181};

void main (void)
{  
   vec3 camPos = inverseViewMatrix[3].xyz;
                                    
   
   float resolutionScalar = 1.0 + clamp(floor(sqrt(camPos.z - WaterHeight) / 5.0), 0.0, 8.0); 
   float ampOverLength = 1.0 / (512.0 * resolutionScalar);

   vec3 textureNormal = vec3(0.0, 0.0, 0.0);  
   for(int i = 0; i < numWaves; ++i)
   {   
      float waveLength = waveLengthArray[i];
   
      float dir = pow(-1.0, float(i)) * float(i) * (textureWaveChopModifier / numWaves);
      float dirAsRad = radians(dir);//radians(waveDirArray[i]);
      //float dirAsRad = radians(waveDirArray[i]);
      float dirCos = cos(dirAsRad);
      float dirSin = sin(dirAsRad);
      vec2 waveDir = vec2(dirSin, dirCos);
      
      float freq = twoPI / waveLength;
      float amp = waveLength * ampOverLength;
      float steepness = 1.0;
      float speed = waveSpeedArray[i];      
   
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

//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

uniform sampler2D reflectionMap;
uniform float elapsedTime;
uniform mat4 inverseViewMatrix;
uniform float textureWaveChopModifier;

const int CURWAVE=0;
const int numWaves = 32;//16;
const float twoPI = 6.283185;

const float waveDirArray[] = {10.33, 3.07, 7.0, 10.246,
                              33.0, 6.71, -104.0, 5.15,
                              -15.5, -21.0, 49.0, 11.0,
                              -9.0, -8.0, 16.113, 26.0};

const float kArray[] = {1.33, 1.76, 2.0, 2.246,
                              1.0, 1.71, 1.0, 1.75,
                              1.5, 1.0, 1.0, 2.0,
                              2.2, 2.0, 1.113, 1.0,
                              1.33, 1.76, 3.0, 2.246,
                              1.0, 1.71, 1.0, 1.75,
                              1.5, 1.0, 1.0, 2.0,
                              1.9, 2.0, 1.113, 1.0};
                  
const float waveLengthArray[] = {0.218, 0.1535, 0.16186, 0.24,
                                 0.19, 0.186844, 0.23437, 0.1805,
                                 0.217, 0.2565, 0.17135 , 0.191,
                                 0.185, 0.23917, 0.215, .248,
                                 0.1788, 0.1835, 0.18186, 0.29,
                                 0.22, 0.246844, 0.27437, 0.1805,
                                 0.197, 0.2565, 0.2735 , 0.291,
                                 0.214, 0.19917, 0.215, .248};
                                  
const float waveSpeedArray[] = {0.0953, 0.083839, -0.0811, 0.08221,
                                -0.11497, 0.143213, 0.14571, 0.091181,
                                
                                0.08473, 0.1331, -0.1731, 0.1021,
                                0.121497, 0.1313, 0.14571, 0.1181,
                                -0.1753, 0.09839, -0.1311, 0.09821,
                                0.09497, 0.093213, 0.13571, 0.121181,
                                
                                -0.10473, 0.0931, 0.13131, -0.0921,
                                0.11497, -0.1313, 0.12571, -0.0931};  

void main (void)
{  
   vec3 camPos = inverseViewMatrix[3].xyz;
                                    
   
   float resolutionScalar = 1.0 + clamp(floor(sqrt(camPos.z) / 5.0), 0.0, 8.0); 
   float ampOverLength = 1.0 / (256.0 * resolutionScalar);

   vec3 textureNormal = vec3(0.0, 0.0, 0.0);  
   for(int i = 0+CURWAVE; i < numWaves+CURWAVE; ++i)
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

      float k = 1.1 * kArray[i];
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

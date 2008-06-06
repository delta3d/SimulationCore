//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

//uniform vec4 lightPos;
//uniform vec4 eyePosition;
//uniform vec4 waterColor;
//uniform vec2 texIncPrev;
//uniform vec2 texInc;
//uniform vec2 texRepeat;

uniform sampler2D normalMap;
uniform float texIncPrevX;
uniform float texIncPrevY;
uniform float texIncX;
uniform float texIncY;
uniform float blend;

varying vec3 lightVector;

void main (void)
{
   //mWaterColorVec = osg::Vec3(0.06f, 0.07f, 0.14f);
   vec4 waterColor = vec4(0.06, 0.07, 0.14, 0.0);
   
   vec2 texIncPrev = vec2(texIncPrevX, texIncPrevY);
   vec2 texInc = vec2(texIncX, texIncY);

   vec2 texCoord1 =  10.0 * gl_TexCoord[0].xy + texIncPrev.xy;
   vec2 texCoord2 =  10.0 * gl_TexCoord[0].xy + texInc.xy;
   
   vec2 texCoord3 = 20.0 * gl_TexCoord[1].xy + texIncPrev.xy;
   vec2 texCoord3_2 = 20.0 * gl_TexCoord[1].xy + texInc.xy;
   
   vec2 texCoord1_1 =  2.0 * gl_TexCoord[0].xy + texIncPrev.xy;
   vec2 texCoord2_2 =  2.0 * gl_TexCoord[0].xy + texInc.xy;      
      
   vec4 bumpColor = mix(texture2D(normalMap, texCoord1) , texture2D(normalMap, texCoord2), blend);
   vec4 bumpColor2 = mix(texture2D(normalMap, texCoord1_1) , texture2D(normalMap, texCoord2_2), blend);
   vec4 bumpColor3 = mix(texture2D(normalMap, texCoord3) , texture2D(normalMap, texCoord3_2), blend);//texture2D(normalMap, texCoord3);
 
   bumpColor *= 2.0;
   bumpColor -= 1.0;
   
   bumpColor2 *= 2.0;
   bumpColor2 -= 1.0;
   
   bumpColor3 *= 2.0;
   bumpColor3 -= 1.0;
   
   vec3 lvts = normalize(lightVector);

   float c = dot(bumpColor.xyz, lvts);
   float c2 = dot(bumpColor2.xyz, lvts);
   float c3 = dot(bumpColor3.xyz, lvts);
   
   vec4 highlight = ((1.0 - c3) * vec4(waterColor.xyz, 1.0));
  
   vec4 finalColor = c * vec4(waterColor.xyz, 1.0);
   vec4 finalColor2 = c2 * vec4(waterColor.xyz, 1.0);
   vec4 diffuse = vec4(0.16, 0.20, 0.26, 1.0);
   
   gl_FragColor = diffuse + 0.5 * highlight + ( 0.5 * (finalColor + finalColor2));
   //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

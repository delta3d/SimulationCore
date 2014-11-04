
uniform sampler2D softGlow;
uniform sampler2D hardGlow;
uniform sampler2D streaks;
uniform sampler2D lastDepthTexture;
uniform vec2 depthTexCoords;

uniform vec2 ScreenDimensions;


void main(void)
{
   //vec4 softGlowTextureColor = texture2D(softGlow, vec2(gl_TexCoord[0].xy));

   vec4 hardGlowTextureColor = texture2D(hardGlow, vec2(gl_TexCoord[0].xy));  
   vec4 streakTextureColor = texture2D(streaks, vec2(gl_TexCoord[0].xy));
    
   float depth = texture2D(lastDepthTexture, depthTexCoords).r;
 
   
   if(depth < 1.0) 
   {
      discard;
   }
   else
   {
      vec4 glowColor = vec4(0.60, 0.60, 0.8, 1.0);
      gl_FragColor =  glowColor * vec4(hardGlowTextureColor.xyz + streakTextureColor.xyz, 1.0);
   }

}

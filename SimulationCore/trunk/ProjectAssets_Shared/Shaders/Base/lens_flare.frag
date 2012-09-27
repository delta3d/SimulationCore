
uniform sampler2D softGlow;
uniform sampler2D hardGlow;
uniform sampler2D streaks;
uniform sampler2D lastDepthTexture;

uniform vec2 ScreenDimensions;


void main(void)
{
   //vec4 softGlowTextureColor = texture2D(softGlow, vec2(gl_TexCoord[0].xy));

   vec4 hardGlowTextureColor = texture2D(hardGlow, vec2(gl_TexCoord[0].xy));
   
   vec4 streakTextureColor = texture2D(streaks, vec2(gl_TexCoord[0].xy));
    
   //just test center pixel, so we either get the whole effect or none of it
   vec2 screenCoord = vec2(0.5, 0.5);
   float depth = texture2D(lastDepthTexture, screenCoord).r;
   
   if(depth < 1.0) 
   {
      discard;
   }
   else
   {
      vec4 glowColor = vec4(0.60f, 0.60f, 0.8f, 1.0f);
      gl_FragColor = glowColor * vec4(hardGlowTextureColor.xyz + streakTextureColor.xyz, 1.0);
   }

}

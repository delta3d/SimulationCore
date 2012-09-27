
uniform sampler2D softGlow;
uniform sampler2D hardGlow;
uniform sampler2D streaks;
uniform sampler2D lastDepthTexture;

uniform vec2 ScreenDimensions;
uniform float effectRadius;

varying vec4 vViewPosCenter;
varying vec4 vViewPosVert;
varying vec3 vPos;
varying vec4 vCenterPixelOfEffect;

void main(void)
{
   //vec4 softGlowTextureColor = texture2D(softGlow, vec2(gl_TexCoord[0].xy));

   vec4 hardGlowTextureColor = texture2D(hardGlow, vec2(gl_TexCoord[0].xy));
   
   vec4 streakTextureColor = texture2D(streaks, vec2(gl_TexCoord[0].xy));
    
   vec2 screenCoord = vec2(0.5, 0.5);//vCenterPixelOfEffect.xy / ScreenDimensions;
   float depth = texture2D(lastDepthTexture, screenCoord).r;
   
   if(depth < 0.9999) 
   {
      discard;
   }
   else
   {
      gl_FragColor = vec4(hardGlowTextureColor.xyz * streakTextureColor.xyz, streakTextureColor.x);
      //gl_FragColor = vec4(depth, depth, depth, 1.0);
   }

}

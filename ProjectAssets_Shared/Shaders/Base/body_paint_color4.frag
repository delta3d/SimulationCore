uniform sampler2D diffuseTexture;
uniform sampler2D PatternTexture;
uniform vec4 ProjectionDir;

uniform vec4 PaintColor1;
uniform vec4 PaintColor2;
uniform vec4 PaintColor3;
uniform vec4 PaintColor4;

varying vec2 vBodyPaintUV;

vec4 getBodyPaintPatternColor()
{
   return texture2D(PatternTexture, vBodyPaintUV);
}

vec4 addBodyPaintColor4(vec4 diffuseColor, vec4 patternColor, float maskingValue)
{
   // Calculate the paint pattern fragment.
   vec4 patColorTemp = patternColor;
   patColorTemp = mix(PaintColor1, mix(PaintColor2, mix(PaintColor3,PaintColor4,patColorTemp.b), patColorTemp.g), patColorTemp.r);
   
   // Modulate the paint pattern fragment to show the original diffuse texture details.
   patColorTemp.xyz += (diffuseColor.xyz - vec3(0.5,0.5,0.5));
   patColorTemp = clamp(patColorTemp, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0));
   
   // Return the modified diffuse color.
   // The fourth component of projection controls whether the effect is fully applied or not.
   float patternEffectOverride = clamp(ProjectionDir.a, 0.0, 1.0);
   return mix(diffuseColor, patColorTemp, maskingValue * patternEffectOverride);
}

vec4 addBodyPaintColor4(vec4 diffuseColor)
{
   // Fetch the Diffuse Color and Color Mask
   vec4 patternColor = getBodyPaintPatternColor();
   return addBodyPaintColor4(diffuseColor, patternColor, diffuseColor.a);
}


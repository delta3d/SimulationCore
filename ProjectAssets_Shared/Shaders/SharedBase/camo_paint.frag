uniform sampler2D diffuseTexture;
uniform sampler2D OverlayTexture;

varying vec2 vDiffuseUVs;
varying vec2 vOverlayUVs;
varying float vOverlayMult;

// EXTERNAL FUNCTIONS
// Functions from: body_paint_color4.frag
vec4 addBodyPaintColor4(vec4 diffuseColor);

vec4 getCamoAndDamageColor()
{
   // Compute the diffuse camo color with appropriate damage state overlay color.
   vec4 diffuseColor = texture2D(diffuseTexture, vDiffuseUVs);
   vec4 overlayColor = texture2D(OverlayTexture, vOverlayUVs);
   diffuseColor = addBodyPaintColor4(diffuseColor);
   return mix(diffuseColor, overlayColor, overlayColor.a * vOverlayMult);
}


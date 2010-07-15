uniform vec4 FrameOffsetAndScales;

varying vec2 vDiffuseUVs;
varying vec2 vOverlayUVs; // For damage state marks.
varying float vOverlayMult;

// EXTERNAL FUNCTIONS
// From: body_paint_color.vert
vec2 calculateBodyPaintUV(vec3 modelVert);

// From: concealment_scale.vert
vec3 getConcealmentScaledVertex(vec3 vert);

void calculateCamoAndDamageUVs()
{
	// Set the diffuse UVs.
   float offset = FrameOffsetAndScales.x;
   vOverlayUVs = vDiffuseUVs = gl_TexCoord[0].st;
//   vDiffuseUVs.t = (vDiffuseUVs.t + offset) * FrameOffsetAndScales.y;
   
   // These lines are for damage states. Determine if the overlay should be visible (no for the non-damaged state).
   vOverlayMult = clamp(max(offset, 0.0), 0.0, 1.0);
   // The fisrt frame of the overlay is for the first state of damage after non-damaged.
   // The diffuse texture should have frames 0 - 2 while the overlay has frames 1 & 2, but not frame 0.
   offset = clamp(offset - 1.0, 0.0, max(0.0, offset));
   
   // Set the overlay UVs.
   vOverlayUVs.t = (vOverlayUVs.t + offset) * FrameOffsetAndScales.z;
   
   // Set the paint pattern UVs.
   calculateBodyPaintUV(getConcealmentScaledVertex(gl_Vertex.xyz));
}
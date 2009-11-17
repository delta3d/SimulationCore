uniform sampler2D diffuseTexture;
uniform float colorOffset;
uniform vec4 buttonInnerHighlightColor;
uniform vec4 buttonOuterHighlightColor;

void main (void)
{
   // Get the primary fragment.
   vec4 frag = texture2D(diffuseTexture, gl_TexCoord[0].st);
   
   // Calculate rotation.
   float rads = radians(colorOffset * 360.0);
   vec2 cosAndSin = vec2(cos(rads), sin(rads));
   
   // Calculate the 1st rotated texture coordinate.
   vec2 lightMaskOffset = gl_TexCoord[0].st + vec2(-0.5,-0.5);
   lightMaskOffset = vec2(lightMaskOffset.s * cosAndSin.x + lightMaskOffset.t * -cosAndSin.y,
	   lightMaskOffset.s * cosAndSin.y + lightMaskOffset.t * cosAndSin.x);
	
	// Get the light mask fragment using the rotated texture coordinate.
   vec2 lightMaskFrag = vec2(texture2D(diffuseTexture, lightMaskOffset + vec2(0.5,0.5)).gg);
   
   // Calculate 2nd the rotated texture coordinate, in the oppsite direction to the first.
   cosAndSin = vec2(cos(rads*2.0), sin(rads*2.0)); // rotate twice as fast.
   lightMaskOffset = gl_TexCoord[0].st + vec2(-0.5,-0.5);
   lightMaskOffset = vec2(lightMaskOffset.s * cosAndSin.x + lightMaskOffset.t * cosAndSin.y,
	   lightMaskOffset.s * -cosAndSin.y + lightMaskOffset.t * cosAndSin.x);
	
	// Get the light mask fragment using the rotated texture coordinate.
   lightMaskFrag.y = texture2D(diffuseTexture, lightMaskOffset + vec2(0.5,0.5)).g;
   
   // Write the combined effect.
   //gl_FragColor = vec4(buttonHighlightColor.rgb,
   //   clamp(lightMaskFrag.x * frag.r + lightMaskFrag.y * frag.b, 0.0, 1.0) * buttonHighlightColor.a * frag.a);
   vec4 innerColor = buttonInnerHighlightColor * lightMaskFrag.x * frag.r; 
   vec4 outerColor = buttonOuterHighlightColor * lightMaskFrag.y * frag.b;
   float alpha = clamp(innerColor.a + outerColor.a, 0.0, 1.0) * frag.a;
   gl_FragColor = vec4(innerColor.rgb + outerColor.rgb, alpha);
}

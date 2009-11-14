uniform sampler2D diffuseTexture;
uniform float colorOffset;
uniform vec4 buttonHighlightColor;

void main (void)
{
   // Get the primary fragment.
   vec4 frag = texture2D(diffuseTexture, gl_TexCoord[0].st);
   
   // Calculate rotation.
   float rads = radians(colorOffset * 360);
   vec2 cosAndSin = vec2(cos(rads), sin(rads));
   
   // Calculate the rotated texture coordinate.
   vec2 lightMaskOffset = gl_TexCoord[0].st + vec2(-0.5,-0.5);
   lightMaskOffset = vec2(lightMaskOffset.s * cosAndSin.x + lightMaskOffset.t * -cosAndSin.y,
	   lightMaskOffset.s * cosAndSin.y + lightMaskOffset.t * cosAndSin.x);
	
	// Get the light mask fragment using the rotated texture coordinate.
   vec4 lightMaskFrag = texture2D(diffuseTexture, lightMaskOffset + vec2(0.5,0.5));
   
   // Write the combined effect.
   gl_FragColor = vec4(buttonHighlightColor.rgb, lightMaskFrag.g * frag.a * buttonHighlightColor.a);
}

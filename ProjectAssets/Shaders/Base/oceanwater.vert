//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

/**
* our uniforms are the light position
* and the eye position 
*/
uniform vec4 lightPos;
uniform vec4 eyePosition;
uniform vec4 waterColor;
uniform vec2 texIncPrev;
uniform vec2 texInc;
uniform float blend;
uniform float texRepeat;
uniform float currentAngle;
varying vec3 lightVector;


void main(void)
{

   vec4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
   
   float second = 5.373 * cos(gl_Vertex.y + currentAngle);
   v.z += second;
	
   float third = 2.5 * cos(gl_Vertex.y + currentAngle + 0.3);
   v.z += third;

   //transform our vector into screen space
   gl_Position = gl_ModelViewProjectionMatrix * v;
    	
   //calculate light vector
   lightVector = lightPos.xyz - v.xyz;
   
   //set output vars
   gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
   gl_TexCoord[1].xy = -vec2(0.0005 * currentAngle, 0.0005 * currentAngle) +  gl_MultiTexCoord0.xy;
   
}





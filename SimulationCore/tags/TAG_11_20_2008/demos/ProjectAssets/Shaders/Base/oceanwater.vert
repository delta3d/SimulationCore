//////////////////////////////////////////////
//A generic ocean water shader
//by Bradley Anderegg
//////////////////////////////////////////////

/**
* our uniforms are the light position
* and the eye position 
*/
//uniform vec4 lightPos;
//uniform vec4 eyePosition;
//uniform float texIncPrevX;
//uniform float texIncPrevY;
///////uniform vec2 texIncPrev;
//uniform float texIncX;
//uniform float texIncY;
//////uniform vec2 texInc;
//uniform vec4 waterColor;
//uniform float texRepeat;
//uniform float blend;

uniform float currentAngle;
varying vec3 lightVector;

void main(void)
{
   vec4 v = vec4( gl_Vertex.x, gl_Vertex.y, gl_Vertex.z, 1.0 );
   
   float second = 0.08 * cos(gl_Vertex.y + currentAngle);
   v.z += second;
	
   float third = .05 * cos(gl_Vertex.y + currentAngle + 0.3);
   v.z += third;

   //transform our vector into screen space
   gl_Position = gl_ModelViewProjectionMatrix * v;
    	
   //calculate light vector
   lightVector = gl_LightSource[0].position.xyz - v.xyz;
   
   //set output vars
   gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
   gl_TexCoord[1].xy = -vec2(0.0005 * currentAngle, 0.0005 * currentAngle) +  gl_MultiTexCoord0.xy;
}





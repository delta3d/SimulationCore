uniform float trailWidth;
uniform mat4 inverseViewMatrix;

void main(void)
{
   vec3 widthNormal = cross(gl_Normal.xyz, gl_Color.xyz) * gl_Color.w;
   widthNormal = normalize(widthNormal);
   widthNormal *= trailWidth * 0.5; // direction;
   
   gl_Position = ftransform() + (gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(widthNormal, 0.0));
   
   gl_TexCoord[0]  = gl_MultiTexCoord0;
}

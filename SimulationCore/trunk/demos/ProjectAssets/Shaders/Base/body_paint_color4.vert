uniform vec4 ProjectionDir;
uniform vec4 ModelDims;

varying vec2 vBodyPaintUV;

vec2 getBodyPaintUV(vec4 projectionDirection)
{
   // Calculate paint projection matrix.
   mat4 patProjMtx = mat4(0.7071,0.0,0.7071,0.0,
   0.0,1.0,0.0,0.0,
   -0.7071,0.0,0.7071,0.0,
   0.0,0.0,0.0,1.0);
   
   vec3 norm = vec3(1.0,1.0,1.0);
   norm = normalize(norm);
   vec3 worldUp = vec3(0.0,0.0,1.0);
   vec3 right = cross(worldUp, norm);
   right = normalize(right);
   vec3 up = cross(norm,right);
   up = normalize(up);
   
   mat4 rot = mat4(right.x,right.y,right.z,0.0,
   norm.x,norm.y,norm.z,0.0,
   up.x,up.y,up.z,0.0,
   0.0,0.0,0.0,1.0);
   patProjMtx = patProjMtx * rot;
   
   return (gl_Vertex * patProjMtx).xy / ModelDims.xy;
}

vec2 calculateBodyPaintUV(vec4 projectionDirection)
{
   vec2 coord = getBodyPaintUV(projectionDirection);
   vBodyPaintUV = coord;
   return coord;
}

vec2 calculateBodyPaintUV()
{
   return calculateBodyPaintUV(ProjectionDir);
}


uniform vec4 ProjectionDir;
uniform vec4 PatternScale;

varying vec2 vBodyPaintUV;

vec2 getBodyPaintUV(vec3 modelVert, vec4 projectionDirection)
{
   // Calculate paint projection matrix.
   vec3 norm = projectionDirection.xyz;
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
   
   vec2 scaleFactor = max(abs(PatternScale.ww * PatternScale.xy), vec2(0.01,0.01)); 
   return (vec4(modelVert,1.0) * rot).xz / scaleFactor;
}

vec2 getBodyPaintUV(vec4 projectionDirection)
{
	return getBodyPaintUV(gl_Vertex.xyz, projectionDirection);
}

vec2 calculateBodyPaintUV(vec3 modelVert, vec4 projectionDirection)
{
   vec2 coord = getBodyPaintUV(modelVert, projectionDirection);
   vBodyPaintUV = coord;
   return coord;
}

vec2 calculateBodyPaintUV(vec4 projectionDirection)
{
   vec2 coord = getBodyPaintUV(gl_Vertex.xyz, projectionDirection);
   vBodyPaintUV = coord;
   return coord;
}

vec2 calculateBodyPaintUV(vec3 modelVert)
{
   vec2 coord = getBodyPaintUV(modelVert, ProjectionDir);
   vBodyPaintUV = coord;
   return coord;
}

vec2 calculateBodyPaintUV()
{
   return calculateBodyPaintUV(gl_Vertex.xyz, ProjectionDir);
}


uniform sampler2D diffuseTexture;
uniform sampler2D PatternTexture;

uniform vec4 ProjectionDir;
uniform vec4 ModelDims;
uniform vec4 PaintColor1;
uniform vec4 PaintColor2;
uniform vec4 PaintColor3;
uniform vec4 PaintColor4;

varying vec4 vModelVertPos;

vec2 getBodyPaintCoordinate(vec4 projectionDirection)
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
   
   return (vModelVertPos * patProjMtx).xy / ModelDims.xy;
}

vec4 getBodyPaintPatternColor(vec4 projectionDirection)
{
   vec2 patSampleCoord = getBodyPaintCoordinate(projectionDirection);
   return texture2D(PatternTexture, patSampleCoord);
}

vec4 addBodyPaintColor4(vec4 diffuseColor, vec4 patternColor, float maskingValue, vec4 projectionDirection)
{
   // Calculate the paint pattern fragment.
   vec4 patColorTemp = patternColor;
   patColorTemp = mix(PaintColor1, mix(PaintColor2, mix(PaintColor3,PaintColor4,patColorTemp.b), patColorTemp.g), patColorTemp.r);
   
   // Modulate the paint pattern fragment to show the original diffuse texture details.
   patColorTemp.xyz += (diffuseColor.xyz - vec3(0.5,0.5,0.5));
   patColorTemp = clamp(patColorTemp, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0));
   
   // Return the modified diffuse color.
   return mix(diffuseColor, patColorTemp, maskingValue);
}

vec4 addBodyPaintColor4(vec4 diffuseColor, vec4 projectionDirection)
{
   // Fetch the Diffuse Color and Color Mask
   vec4 patternColor = getBodyPaintPatternColor(projectionDirection);
   return addBodyPaintColor4(diffuseColor, patternColor, diffuseColor.a, projectionDirection);
}

